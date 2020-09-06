#include "BLE.h"

#include "Event.h"

#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")
#define TO_SEARCH_DEVICE_UUID                                                                      \
    "{00001623-1212-EFDE-1623-785FEABCD123}" //we use UUID for an HR BLE device

#include <memory>

#include "spdlog_wrap.h"

namespace Ancorage::BLE
{
class BLEManager::Impl
{
public:
    ~Impl();
    bool Connect();
    bool Run();
    void Stop();

private:
    void threadProc();
    bool init();
    void onEvent(BTH_LE_GATT_EVENT_TYPE, void* param);
    static void event_callback(BTH_LE_GATT_EVENT_TYPE type, void* param, void* data)
    {
        auto p = static_cast<BLEManager::Impl*>(data);
        p->onEvent(type, param);
    }

    HANDLE handle_ = nullptr;
    std::thread t_;
    std::atomic<bool> running_{true};
    PBTH_LE_GATT_CHARACTERISTIC currGattChar_ = nullptr;
};

BLEManager::Impl::~Impl() = default;

bool BLEManager::Impl::Connect()
{
    auto guidStr = L"{00001623-1212-EFDE-1623-785FEABCD123}";
    GUID guid;
    auto hr = CLSIDFromString(guidStr, &guid);
    if (hr != S_OK) {
        spdlog::error("Device GUID incorrect");
        return false;
    }

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
        auto ret = SetupDiEnumDeviceInterfaces(hDI, NULL, &guid, deviceIdx, &did);
        if (!ret) {
            if (GetLastError() != ERROR_NO_MORE_ITEMS) {
                return false;
            }
            break;
        }

        DWORD size = 0;
        PSP_DEVICE_INTERFACE_DETAIL_DATA didd = nullptr;

        if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, nullptr, size, &size, 0)) {
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

            handle_ = CreateFile(didd->DevicePath, GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            break;
        }
        ++deviceIdx;
    }

    return true;
}

bool BLEManager::Impl::Run()
{
    if (!init()) {
        return false;
    }
    t_ = std::thread(&BLEManager::Impl::threadProc, this);
    return true;
}

void BLEManager::Impl::Stop()
{
    running_ = false;
    if (t_.joinable()) {
        t_.join();
    }
}

void BLEManager::Impl::onEvent(BTH_LE_GATT_EVENT_TYPE, void* param)
{
    auto params = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)param;
    auto message =
        Message::Parse(params->CharacteristicValue->Data, params->CharacteristicValue->DataSize);
    if (message) {
        spdlog::info("Message: {}", message->ToString());
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
    HRESULT hr = BluetoothGATTGetServices(handle_, 0, NULL, &bufCount, BLUETOOTH_GATT_FLAG_NONE);
    if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        return false;
    }
    Buffer<BTH_LE_GATT_SERVICE> serviceBuf(sizeof(BTH_LE_GATT_SERVICE) * bufCount);
    USHORT numServices;
    hr = BluetoothGATTGetServices(
        handle_, bufCount, serviceBuf, &numServices, BLUETOOTH_GATT_FLAG_NONE);

    if (hr != S_OK) {
        return false;
    }

    //Step 3: now get the list of charactersitics. note how the pServiceBuffer is required from step 2
    ////////////////////////////////////////////////////////////////////////////
    // Determine Characteristic Buffer Size
    ////////////////////////////////////////////////////////////////////////////

    USHORT charBufferSize;
    hr = BluetoothGATTGetCharacteristics(
        handle_, serviceBuf, 0, NULL, &charBufferSize, BLUETOOTH_GATT_FLAG_NONE);

    if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        return false;
    }
    if (charBufferSize != 1) {
        spdlog::error("Expected 1 BLE characteristic. Got {}", charBufferSize);
        return false;
    }

    Buffer<BTH_LE_GATT_CHARACTERISTIC> charBuf(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
    USHORT numChars;
    hr = BluetoothGATTGetCharacteristics(
        handle_, serviceBuf, charBufferSize, charBuf, &numChars, BLUETOOTH_GATT_FLAG_NONE);

    if (hr != S_OK) {
        return false;
    }

    if (numChars != charBufferSize) {
        return false;
    }
    //Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
    //descriptors are required as we descriptors that are notification based will have to be written
    //once IsSubcribeToNotification set to true, we set the appropriate callback function
    //need for setting descriptors for notification according to
    //http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
    currGattChar_ = &charBuf[0];
    USHORT charValueDataSize;

    ///////////////////////////////////////////////////////////////////////////
    // Determine Descriptor Buffer Size
    ////////////////////////////////////////////////////////////////////////////
    USHORT descriptorBufferSize;
    hr = BluetoothGATTGetDescriptors(
        handle_, currGattChar_, 0, NULL, &descriptorBufferSize, BLUETOOTH_GATT_FLAG_NONE);

    if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
        return false;
    }

    if (descriptorBufferSize > 0) {
        Buffer<BTH_LE_GATT_DESCRIPTOR> descriptorBuf(
            descriptorBufferSize * sizeof(BTH_LE_GATT_DESCRIPTOR));

        ////////////////////////////////////////////////////////////////////////////
        // Retrieve Descriptors
        ////////////////////////////////////////////////////////////////////////////

        USHORT numDescriptors;
        hr = BluetoothGATTGetDescriptors(handle_, currGattChar_, descriptorBufferSize,
            descriptorBuf, &numDescriptors, BLUETOOTH_GATT_FLAG_NONE);

        if (hr != S_OK) {
            return false;
        }

        if (numDescriptors != descriptorBufferSize) {
            return false;
        }

        for (int kk = 0; kk < numDescriptors; kk++) {
            auto currGattDescriptor = &descriptorBuf[kk];
            ////////////////////////////////////////////////////////////////////////////
            // Determine Descriptor Value Buffer Size
            ////////////////////////////////////////////////////////////////////////////
            USHORT descValueDataSize;
            hr = BluetoothGATTGetDescriptorValue(
                handle_, currGattDescriptor, 0, NULL, &descValueDataSize, BLUETOOTH_GATT_FLAG_NONE);

            if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
                return false;
            }

            Buffer<BTH_LE_GATT_DESCRIPTOR_VALUE> pDescValueBuffer(descValueDataSize);
            ////////////////////////////////////////////////////////////////////////////
            // Retrieve the Descriptor Value
            ////////////////////////////////////////////////////////////////////////////

            hr = BluetoothGATTGetDescriptorValue(handle_, currGattDescriptor,
                (ULONG)descValueDataSize, pDescValueBuffer, NULL, BLUETOOTH_GATT_FLAG_NONE);
            if (hr != S_OK) {
                return false;
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
                if (hr != S_OK) {
                    return false;
                } else {
                    spdlog::info("Setting notification for serivice handle {}",
                        static_cast<int>(currGattDescriptor->ServiceHandle));
                }
            }
        }
    }

    //set the appropriate callback function when the descriptor change value
    BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

    if (currGattChar_->IsNotifiable) {
        spdlog::info("Setting Notification for ServiceHandle {}",
            static_cast<int>(currGattChar_->ServiceHandle));
        BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

        BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
        EventParameterIn.Characteristics[0] = *currGattChar_;
        EventParameterIn.NumCharacteristics = 1;
        hr = BluetoothGATTRegisterEvent(handle_, EventType, &EventParameterIn,
            &BLEManager::Impl::event_callback, this, &EventHandle, BLUETOOTH_GATT_FLAG_NONE);

        if (hr != S_OK) {
            return false;
        }
    }

    if (currGattChar_->IsReadable) { //currGattChar->IsReadable
        ////////////////////////////////////////////////////////////////////////////
        // Determine Characteristic Value Buffer Size
        ////////////////////////////////////////////////////////////////////////////
        hr = BluetoothGATTGetCharacteristicValue(
            handle_, currGattChar_, 0, NULL, &charValueDataSize, BLUETOOTH_GATT_FLAG_NONE);

        if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            return false;
        }

        Buffer<BTH_LE_GATT_CHARACTERISTIC_VALUE> pCharValueBuffer(charValueDataSize);

        ////////////////////////////////////////////////////////////////////////////
        // Retrieve the Characteristic Value
        ////////////////////////////////////////////////////////////////////////////

        hr = BluetoothGATTGetCharacteristicValue(handle_, currGattChar_, (ULONG)charValueDataSize,
            pCharValueBuffer, NULL, BLUETOOTH_GATT_FLAG_NONE);

        if (hr != S_OK) {
            return false;
        }

        //print the characeteristic Value
        //for an HR monitor this might be the body sensor location
        /*
        printf("\n Printing a read (not notifiable) characterstic (maybe) body sensor value");
        for (uint32_t iii = 0; iii < pCharValueBuffer->DataSize;
             iii++) { // ideally check ->DataSize before printing
            printf(" %x", pCharValueBuffer->Data[iii]);
        }
        printf("\n");
        */
        // Free before going to next iteration, or memory leak.
    }

    return true;
}

void BLEManager::Impl::threadProc()
{
    HRESULT hr = S_OK;
    //go into an inf loop that sleeps. you will ideally see notifications from the HR device
    int bakabaka = 0;
    while (running_) {
        Sleep(1000);
        if (bakabaka == 1) {
            bakabaka = 0;
            BTH_LE_GATT_RELIABLE_WRITE_CONTEXT ReliableWriteContext = NULL;
            hr = BluetoothGATTBeginReliableWrite(
                handle_, &ReliableWriteContext, BLUETOOTH_GATT_FLAG_NONE);

            if (SUCCEEDED(hr)) {
                BTH_LE_GATT_CHARACTERISTIC_VALUE* newValue =
                    (BTH_LE_GATT_CHARACTERISTIC_VALUE*)malloc(6);

                RtlZeroMemory(newValue, 6);
                UCHAR valueData[] = {0x4, 0x0, 0x02, 0x01};
                newValue->DataSize = valueData[0];

                memcpy(newValue->Data, (UCHAR*)valueData, valueData[0]);

                // Set the new characteristic value
                hr = BluetoothGATTSetCharacteristicValue(
                    handle_, currGattChar_, newValue, NULL, BLUETOOTH_GATT_FLAG_NONE);
            }

            if (NULL != ReliableWriteContext) {
                BluetoothGATTEndReliableWrite(
                    handle_, ReliableWriteContext, BLUETOOTH_GATT_FLAG_NONE);
            }
        } else if (bakabaka == 2) {
            break;
        }
        //printf("look for notification");
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

BLEManager::~BLEManager() = default;

bool BLEManager::Connect()
{
    return impl_->Connect();
}

void BLEManager::Run()
{
    impl_->Run();
}

void BLEManager::Stop()
{
    impl_->Stop();
}
} // namespace Ancorage::BLE
