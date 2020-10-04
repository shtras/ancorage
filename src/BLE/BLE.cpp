#include "BLE.h"

#include "Message.h"
#include "MessageFactory.h"
#include "Utils/Utils.h"

#include "spdlog_wrap.h"

#include "Windows_wrap.h"
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
constexpr char TO_SEARCH_DEVICE_UUID[] = "{00001623-1212-EFDE-1623-785FEABCD123}";

#include <memory>
#include <list>
#include <mutex>

namespace Ancorage::BLE
{
class BLEManager::Impl
{
public:
    ~Impl();
    bool Connect(const std::wstring& id);
    bool Run();
    void Stop();
    void SendBTMessage(const std::shared_ptr<Message>& m);
    void SetSink(Sink* sink);

private:
    void threadProc();
    bool init();
    void onEvent(BTH_LE_GATT_EVENT_TYPE, void* param);
    static void CALLBACK eventCallback(BTH_LE_GATT_EVENT_TYPE type, void* param, void* data)
    {
        auto p = static_cast<BLEManager::Impl*>(data);
        p->onEvent(type, param);
    }

    std::wstring id_;
    HANDLE handle_ = nullptr;
    std::thread t_;
    std::atomic<bool> running_{false};
    PBTH_LE_GATT_CHARACTERISTIC currGattChar_ = nullptr;
    BLUETOOTH_GATT_EVENT_HANDLE eventHandle_ = nullptr;
    mutable Utils::Semaphore s_;
    mutable std::mutex m_;
    std::list<std::shared_ptr<Message>> messages_;
    Sink* sink_ = nullptr;
    std::atomic<bool> connected_{false};
};

BLEManager::Impl::~Impl() = default;

bool BLEManager::Impl::Connect(const std::wstring& id)
{
    auto guidStr = L"{00001623-1212-EFDE-1623-785FEABCD123}";
    GUID guid;
    auto hr = CLSIDFromString(guidStr, &guid);
    if (hr != S_OK) {
        spdlog::error("Device GUID incorrect");
        return false;
    }
    spdlog::info("Connection started");
    auto hDI = SetupDiGetClassDevs(&guid, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    auto raii = std::shared_ptr<int>(nullptr, [&](int*) { SetupDiDestroyDeviceInfoList(hDI); });

    if (hDI == INVALID_HANDLE_VALUE) {
        return false;
    }
    SP_DEVICE_INTERFACE_DATA did;
    did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    SP_DEVINFO_DATA dd;
    dd.cbSize = sizeof(SP_DEVINFO_DATA);

    DWORD deviceIdx = 0;
    for (;;) {
        auto ret = SetupDiEnumDeviceInterfaces(hDI, nullptr, &guid, deviceIdx, &did);
        if (!ret) {
            if (GetLastError() != ERROR_NO_MORE_ITEMS) {
                return false;
            }
            break;
        }

        DWORD size = 0;
        PSP_DEVICE_INTERFACE_DETAIL_DATA didd = nullptr;

        if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, nullptr, size, &size, nullptr)) {
            auto err = GetLastError();

            if (err != ERROR_INSUFFICIENT_BUFFER) {
                return false;
            }

            didd = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new byte[size];
            ZeroMemory(didd, size);
            auto diddRaii = std::shared_ptr<int>(nullptr, [&](int*) { delete[](didd); });

            didd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, didd, size, &size, &dd)) {
                return false;
            }

            std::wstring devicePath{didd->DevicePath};
            if (devicePath.find(id) != std::wstring::npos) {
                handle_ = CreateFile(didd->DevicePath, GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
                break;
            }
            ++deviceIdx;
        }
    }
    if (!handle_) {
        spdlog::error("Device not found");
        return false;
    }
    spdlog::info("Connection complete");
    connected_ = true;
    return true;
}

bool BLEManager::Impl::Run()
{
    spdlog::info("Initialzing");
    if (!init()) {
        spdlog::error("Init failed");
        return false;
    }
    spdlog::info("Init complete");
    running_ = true;
    t_ = std::thread(&BLEManager::Impl::threadProc, this);
    return true;
}

void BLEManager::Impl::Stop()
{
    if (!running_) {
        return;
    }
    running_ = false;
    {
        std::lock_guard l(m_);

        auto hr = BluetoothGATTUnregisterEvent(eventHandle_, BLUETOOTH_GATT_FLAG_NONE);
        if (FAILED(hr)) {
            spdlog::error("Unregister event failed: {0:x}", -hr);
        }

        auto shutdownM =
            MessageFactory::CreateHubActionsMessage(HubActionsMessage::ActionType::SwitchOff);
        messages_.push_back(shutdownM);
    }
    s_.notify();
    if (t_.joinable()) {
        t_.join();
    }
}

void BLEManager::Impl::SetSink(Sink* sink)
{
    sink_ = sink;
}

void BLEManager::Impl::onEvent(BTH_LE_GATT_EVENT_TYPE, void* param)
{
    auto params = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)param;
    auto message =
        Message::Parse(params->CharacteristicValue->Data, params->CharacteristicValue->DataSize);
    if (message) {
        if (message->GetType() == Message::Type::HubAttachedIO) {
            spdlog::debug("Received message: {}", message->ToString());
        }
        if (sink_) {
            sink_->Consume(message);
        }
    } else {
        spdlog::error("Error:(");
    }
}

template <typename T>
class Buffer
{
public:
    explicit Buffer(size_t s)
    {
        buf_ = reinterpret_cast<T*>(new char[s]);
        ZeroMemory(buf_, s);
    }
    ~Buffer()
    {
        delete[] buf_;
    }
    T* get()
    {
        return buf_;
    }
    operator T*()
    {
        return buf_;
    }
    T* operator->()
    {
        return buf_;
    }

private:
    T* buf_;
};

bool BLEManager::Impl::init()
{
    //Step 2: Get a list of services that the device advertises
    // first send 0,NULL as the parameters to BluetoothGATTServices inorder to get the number of
    // services in serviceBufferCount
    USHORT bufCount = 0;
    HRESULT hr = BluetoothGATTGetServices(handle_, 0, nullptr, &bufCount, BLUETOOTH_GATT_FLAG_NONE);
    if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        spdlog::error("GetSetvices1 failed: {0:x}", -hr);
        return false;
    }
    Buffer<BTH_LE_GATT_SERVICE> serviceBuf(sizeof(BTH_LE_GATT_SERVICE) * bufCount);
    USHORT numServices = 0;
    hr = BluetoothGATTGetServices(
        handle_, bufCount, serviceBuf, &numServices, BLUETOOTH_GATT_FLAG_NONE);

    if (FAILED(hr)) {
        spdlog::error("GetSetvices2 failed: {0:x}", -hr);
        return false;
    }

    //Step 3: now get the list of charactersitics. note how the pServiceBuffer is required from step 2
    ////////////////////////////////////////////////////////////////////////////
    // Determine Characteristic Buffer Size
    ////////////////////////////////////////////////////////////////////////////

    USHORT charBufferSize = 0;
    hr = BluetoothGATTGetCharacteristics(
        handle_, serviceBuf, 0, nullptr, &charBufferSize, BLUETOOTH_GATT_FLAG_NONE);

    if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        spdlog::error("GetCharacteristics1 failed: {0:x}", -hr);
        return false;
    }
    if (charBufferSize != 1) {
        spdlog::error("Expected 1 BLE characteristic. Got {}", charBufferSize);
        return false;
    }

    Buffer<BTH_LE_GATT_CHARACTERISTIC> charBuf(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
    USHORT numChars = 0;
    hr = BluetoothGATTGetCharacteristics(
        handle_, serviceBuf, charBufferSize, charBuf, &numChars, BLUETOOTH_GATT_FLAG_NONE);

    if (FAILED(hr)) {
        spdlog::error("GetCharacteristics2 failed: {0:x}", -hr);
        return false;
    }

    if (numChars != charBufferSize) {
        spdlog::error("Buffer size mismatch");
        return false;
    }
    //Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
    //descriptors are required as we descriptors that are notification based will have to be written
    //once IsSubcribeToNotification set to true, we set the appropriate callback function
    //need for setting descriptors for notification according to
    //http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
    currGattChar_ = new BTH_LE_GATT_CHARACTERISTIC;
    *currGattChar_ = charBuf[0];
    //USHORT charValueDataSize = 0;

    ///////////////////////////////////////////////////////////////////////////
    // Determine Descriptor Buffer Size
    ////////////////////////////////////////////////////////////////////////////
    USHORT descriptorBufferSize = 0;
    hr = BluetoothGATTGetDescriptors(
        handle_, currGattChar_, 0, nullptr, &descriptorBufferSize, BLUETOOTH_GATT_FLAG_NONE);

    if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        spdlog::error("GetDescriptors1 failed: {0:x}", -hr);
        return false;
    }

    if (descriptorBufferSize > 0) {
        Buffer<BTH_LE_GATT_DESCRIPTOR> descriptorBuf(
            descriptorBufferSize * sizeof(BTH_LE_GATT_DESCRIPTOR));

        ////////////////////////////////////////////////////////////////////////////
        // Retrieve Descriptors
        ////////////////////////////////////////////////////////////////////////////

        USHORT numDescriptors = 0;
        hr = BluetoothGATTGetDescriptors(handle_, currGattChar_, descriptorBufferSize,
            descriptorBuf, &numDescriptors, BLUETOOTH_GATT_FLAG_NONE);

        if (FAILED(hr)) {
            spdlog::error("GetDescriptors2 failed: {0:x}", -hr);
            return false;
        }

        if (numDescriptors != descriptorBufferSize) {
            spdlog::error("Descriptors count mismatch");
            return false;
        }

        for (int kk = 0; kk < numDescriptors; kk++) {
            auto currGattDescriptor = &descriptorBuf[kk];
            ////////////////////////////////////////////////////////////////////////////
            // Determine Descriptor Value Buffer Size
            ////////////////////////////////////////////////////////////////////////////
            USHORT descValueDataSize = 0;
            hr = BluetoothGATTGetDescriptorValue(handle_, currGattDescriptor, 0, nullptr,
                &descValueDataSize, BLUETOOTH_GATT_FLAG_NONE);

            if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
                spdlog::error("GetDescriptorValue1 failed: {0:x}", -hr);
                //return false;
            }

            Buffer<BTH_LE_GATT_DESCRIPTOR_VALUE> pDescValueBuffer(descValueDataSize);
            ////////////////////////////////////////////////////////////////////////////
            // Retrieve the Descriptor Value
            ////////////////////////////////////////////////////////////////////////////

            hr = BluetoothGATTGetDescriptorValue(handle_, currGattDescriptor,
                (ULONG)descValueDataSize, pDescValueBuffer, nullptr, BLUETOOTH_GATT_FLAG_NONE);
            if (FAILED(hr)) {
                spdlog::error("GetDescriptorValue2 failed: {0:x}", -hr);
                //return false;
            }
            //you may also get a descriptor that is read (and not notify) andi am guessing the attribute handle is out of limits
            // we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
            if (currGattDescriptor->AttributeHandle < 255) {
                BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

                RtlZeroMemory(&newValue, sizeof(newValue));

                newValue.DescriptorType = ClientCharacteristicConfiguration;
                newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

                hr = BluetoothGATTSetDescriptorValue(
                    handle_, currGattDescriptor, &newValue, BLUETOOTH_GATT_FLAG_NONE);
                if (FAILED(hr)) {
                    spdlog::error("SetDescriptorValue failed: {0:x}", -hr);
                    //return false;
                } else {
                    spdlog::info("Setting notification for serivice handle {}",
                        static_cast<int>(currGattDescriptor->ServiceHandle));
                }
            }
        }
    }

    //set the appropriate callback function when the descriptor change value

    if (currGattChar_->IsNotifiable) {
        spdlog::info("Setting Notification for ServiceHandle {}",
            static_cast<int>(currGattChar_->ServiceHandle));
        BTH_LE_GATT_EVENT_TYPE eventType = CharacteristicValueChangedEvent;

        BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION eventParameterIn;
        eventParameterIn.Characteristics[0] = *currGattChar_;
        eventParameterIn.NumCharacteristics = 1;
        hr = BluetoothGATTRegisterEvent(handle_, eventType, &eventParameterIn,
            &BLEManager::Impl::eventCallback, this, &eventHandle_, BLUETOOTH_GATT_FLAG_NONE);

        if (FAILED(hr)) {
            spdlog::error("RegisterEvent failed: {0:x}", -hr);
            return false;
        }
    }
    return true;
    /*
    if (currGattChar_->IsReadable) { //currGattChar->IsReadable
        ////////////////////////////////////////////////////////////////////////////
        // Determine Characteristic Value Buffer Size
        ////////////////////////////////////////////////////////////////////////////
        hr = BluetoothGATTGetCharacteristicValue(
            handle_, currGattChar_, 0, nullptr, &charValueDataSize, BLUETOOTH_GATT_FLAG_NONE);
        if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            spdlog::error("GetCharacteristicValue1 failed: {0:x}", -hr);
            //return false;
        }

        Buffer<BTH_LE_GATT_CHARACTERISTIC_VALUE> pCharValueBuffer(charValueDataSize);

        ////////////////////////////////////////////////////////////////////////////
        // Retrieve the Characteristic Value
        ////////////////////////////////////////////////////////////////////////////

        hr = BluetoothGATTGetCharacteristicValue(handle_, currGattChar_, (ULONG)charValueDataSize,
            pCharValueBuffer, nullptr, BLUETOOTH_GATT_FLAG_NONE);

        if (FAILED(hr)) {
            spdlog::error("GetCharacteristicValue2 failed: {0:x}", -hr);
            //return false;
        }

        auto message = Message::Parse(pCharValueBuffer->Data, pCharValueBuffer->DataSize);
        if (message) {
            spdlog::info("Initial message: {}", message->ToString());
        } else {
            spdlog::error("Error:(");
        }
    }

    return true;*/
}

void BLEManager::Impl::SendBTMessage(const std::shared_ptr<Message>& m)
{
    if (!connected_) {
        return;
    }
    if (!running_) {
        spdlog::warn("Can't send message. Stopping...");
        return;
    }
    std::lock_guard l(m_);
    messages_.push_back(m);
    s_.notify();
}

void BLEManager::Impl::threadProc()
{
    while (running_) {
        s_.wait();
        while (!messages_.empty()) {
            {
                std::lock_guard l(m_);
                const auto& msg = messages_.front();
                spdlog::info("Sending message: {}", msg->ToString());
                auto buf = msg->ToBuffer();
                BTH_LE_GATT_RELIABLE_WRITE_CONTEXT writeCtx = 0;
                auto hr =
                    BluetoothGATTBeginReliableWrite(handle_, &writeCtx, BLUETOOTH_GATT_FLAG_NONE);

                if (FAILED(hr) || !writeCtx) {
                    break;
                }
                Buffer<BTH_LE_GATT_CHARACTERISTIC_VALUE> newValue(buf.size() + sizeof(ULONG));

                newValue->DataSize = static_cast<ULONG>(buf.size());
                memcpy(newValue->Data, (UCHAR*)buf.data(), buf.size());

                hr = BluetoothGATTSetCharacteristicValue(
                    handle_, currGattChar_, newValue, 0, BLUETOOTH_GATT_FLAG_NONE);

                if (writeCtx) {
                    BluetoothGATTEndReliableWrite(handle_, writeCtx, BLUETOOTH_GATT_FLAG_NONE);
                }
                messages_.pop_front();
            }
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    CloseHandle(handle_);
    auto err = GetLastError();
    if (err != NO_ERROR && err != ERROR_NO_MORE_ITEMS) {
        spdlog::error("Failed to close handle");
    }
}

BLEManager::BLEManager()
    : impl_(std::make_unique<Impl>())
{
}

BLEManager::~BLEManager()
{
    impl_->Stop();
};

bool BLEManager::Connect(const std::wstring& id)
{
    return impl_->Connect(id);
}

void BLEManager::Run()
{
    impl_->Run();
}

void BLEManager::Stop()
{
    impl_->Stop();
    impl_ = std::make_unique<Impl>();
}

void BLEManager::SendBTMessage(const std::shared_ptr<Message>& m)
{
    impl_->SendBTMessage(m);
}

void BLEManager::SetSink(Sink* sink)
{
    impl_->SetSink(sink);
}
} // namespace Ancorage::BLE
