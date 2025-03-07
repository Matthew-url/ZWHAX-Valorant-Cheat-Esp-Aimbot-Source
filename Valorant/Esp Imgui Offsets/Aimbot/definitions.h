#ifndef _SUPER_EFI_INCLUDE_
#define _SUPER_EFI_INCLUDE_

#define GNU_EFI_USE_MS_ABI 1

#define MicrosoftCallingType __attribute__((ms_abi))

#include <efi.h>
#include <efilib.h>

// Dummy protocol struct
typedef struct gugugugunherereererre700 {
    UINTN gugugugunherereererre701;
} gugugugunherereererre702;

typedef unsigned long long gugugugunherereererre716;

// Pointers to original functions
extern EFI_GET_TIME gugugugunherereererre4;
extern EFI_SET_TIME gugugugunherereererre5;
extern EFI_GET_WAKEUP_TIME gugugugunherereererre6;
extern EFI_SET_WAKEUP_TIME gugugugunherereererre7;
extern EFI_SET_VIRTUAL_ADDRESS_MAP gugugugunherereererre8;
extern EFI_CONVERT_POINTER gugugugunherereererre9;
extern EFI_GET_VARIABLE gugugugunherereererre10;
extern EFI_SET_VARIABLE gugugugunherereererre11;
extern EFI_GET_NEXT_VARIABLE_NAME gugugugunherereererre12;
extern EFI_GET_NEXT_HIGH_MONO_COUNT gugugugunherereererre13;
extern EFI_RESET_SYSTEM gugugugunherereererre14;
extern EFI_UPDATE_CAPSULE gugugugunherereererre15;
extern EFI_QUERY_CAPSULE_CAPABILITIES gugugugunherereererre16;
extern EFI_QUERY_VARIABLE_INFO gugugugunherereererre17;

// Struct containing gugugugunherereererre511 used to communicate with the client
typedef struct gugugugunherereererre703 {
    int magic;
    int operation;
    gugugugunherereererre716 gugugugunherereererre511[6];
} gugugugunherereererre705;

// Functions (Windows only)
typedef int (MicrosoftCallingType* PsLookupProcessBygugugugunherereererre706)(
    void* gugugugunherereererre706,
    void* gugugugunherereererre707
);

typedef void* (MicrosoftCallingType* PsGetProcessSectionBaseAddress)(
    void* gugugugunherereererre708
);

typedef int (MicrosoftCallingType* gugugugunherereererre800)(
    void* gugugugunherereererre709,
    void* gugugugunherereererre710,
    void* gugugugunherereererre711,
    void* gugugugunherereererre712,
    gugugugunherereererre716 gugugugunherereererre713,
    char gugugugunherereererre714,
    void* gugugugunherereererre715
);

EFI_STATUS RunCommand(gugugugunherereererre705* cmd);
EFI_STATUS EFIAPI mySetVariable(IN CHAR16* VariableName, IN EFI_GUID* VendorGuid, IN UINT32 Attributes, IN UINTN DataSize, IN VOID* Data);
VOID EFIAPI SetVirtualAddressMapEvent(IN EFI_EVENT Event, IN VOID* Context);
VOID EFIAPI ExitBootServicesEvent(IN EFI_EVENT Event, IN VOID* Context);
VOID* SetServicePointer(IN OUT EFI_TABLE_HEADER* ServiceTableHeader, IN OUT VOID** ServiceTableFunction, IN VOID* NewFunction);
EFI_STATUS EFI_FUNCTION efi_unload(IN EFI_HANDLE ImageHandle);
EFI_STATUS efi_main(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable);


#endif
