#include <stdint.h>

#ifndef _STDINT_H
#define _STDINT_H
#endif

#include <kernel.h>
#include <uefi.h>

extern BootInfo g_boot_info;

// MP Services GUID
static efi_guid_t mp_services_guid = {
    0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};

// CPU Core Location Struct
typedef struct {
  uint32_t Package;
  uint32_t Core;
  uint32_t Thread;
} efi_cpu_physical_location_t;

// EFI_PROCESSOR_INFORMATION Struct
typedef struct {
  uint64_t ProcessorId; // Hardware LAPIC ID
  uint32_t StatusFlag;
  efi_cpu_physical_location_t Location;
  // Extended information fields (omitted/reserved for basic usage)
} efi_processor_information_t;

// Protocol Function Definitions
typedef struct efi_mp_services_protocol efi_mp_services_protocol_t;

typedef efi_status_t(EFIAPI *efi_mp_services_get_number_of_processors)(
    efi_mp_services_protocol_t *This, uintn_t *NumberOfProcessors,
    uintn_t *NumberOfEnabledProcessors);

typedef efi_status_t(EFIAPI *efi_mp_services_get_processor_information)(
    efi_mp_services_protocol_t *This, uintn_t ProcessorNumber,
    efi_processor_information_t *ProcessorInformationBuffer);

// MP Services Protocol
struct efi_mp_services_protocol {
  efi_mp_services_get_number_of_processors GetNumberOfProcessors;
  efi_mp_services_get_processor_information GetProcessorInformation;
  void *StartupAllAPs;
  void *StartupThisAP;
  void *SwitchBSP;
  void *EnableDisableAP;
  void *WhoAmI;
};

// Detect CPU Cores and fetch true LAPIC IDs
void init_cpu_cores() {
  efi_mp_services_protocol_t *mp_services = NULL;
  efi_status_t status = BS->LocateProtocol(&mp_services_guid, NULL, (void **)&mp_services);

  uintn_t total_cpus = 1;

  if (!EFI_ERROR(status) && mp_services != NULL && mp_services->GetNumberOfProcessors != NULL) {
    uintn_t enabled_cpus = 0;
    status = mp_services->GetNumberOfProcessors(mp_services, &total_cpus, &enabled_cpus);
    if (EFI_ERROR(status)) {
      total_cpus = 1;
    }
  }

  g_boot_info.g_cpu_count = (uint32_t)total_cpus;
  g_boot_info.g_cpus = malloc(g_boot_info.g_cpu_count * sizeof(CpuCore));

  // Fetch hardware LAPIC ID for all detected core
  for (uint32_t i = 0; i < g_boot_info.g_cpu_count; i++) {
    efi_processor_information_t proc_info;

    if (mp_services != NULL && mp_services->GetProcessorInformation != NULL) {
      status = mp_services->GetProcessorInformation(mp_services, i, &proc_info);
    } else {
      status = EFI_UNSUPPORTED;
    }

    if (!EFI_ERROR(status)) {
      g_boot_info.g_cpus[i].lapic_id = (uint32_t)proc_info.ProcessorId;
    } else {
      g_boot_info.g_cpus[i].lapic_id = i;
    }
  }
}
