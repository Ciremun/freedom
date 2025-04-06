#include "lazer/sharp.h"

#pragma pack(push, 1)
typedef struct {
    uint32_t Signature;
    uint16_t MajorVersion;
    uint16_t MinorVersion;
    uint32_t Reserved;
    uint32_t VersionLength;
    uint16_t Flags;
    uint16_t Streams;
} METADATA_HEADER;

typedef struct {
    uint32_t Offset;
    uint32_t Size;
} STREAM_HEADER;

typedef struct {
    uint32_t Reserved;
    uint8_t MajorVersion;
    uint8_t MinorVersion;
    uint8_t HeapSizes;
    uint8_t Reserved2;
    uint64_t Valid;
    uint64_t Sorted;
} TABLES_HEADER;
#pragma pack(pop)

static inline size_t table_row_size(int table_idx, uint8_t heap_sizes)
{
    bool string_heap = (heap_sizes & 0x01) != 0;
    bool guid_heap = (heap_sizes & 0x02) != 0;
    bool blob_heap = (heap_sizes & 0x04) != 0;
    switch (table_idx)
    {
        // Module - 2 bytes generation, 4 bytes name, 3 GUIDs
        case 0x00: return 2 + (string_heap ? 4 : 2) + (guid_heap ? 12 : 6);
        // TypeRef - Resolution scope + name + namespace
        case 0x01: return 2 + (string_heap ? 4 : 2) + (string_heap ? 4 : 2);
        // TypeDef - Flags (4), name, namespace, extends, field list, method list
        case 0x02: return 4 + (string_heap ? 4 : 2) + (string_heap ? 4 : 2) + 2 + 2 + 2;
        // Field - Flags (2), name, signature
        case 0x04: return 2 + (string_heap ? 4 : 2) + (blob_heap ? 4 : 2);
        default: return 0;
    }
}

uint32_t *token_to_rva(uintptr_t base, uint32_t token)
{
    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)base;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    IMAGE_NT_HEADERS64 *nt_headers = (IMAGE_NT_HEADERS64 *)((uint8_t *)base + dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    uint32_t cli_rva = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress;
    if (!cli_rva)
        return 0;

    IMAGE_COR20_HEADER *cli_header = (IMAGE_COR20_HEADER *)((uint8_t *)base + cli_rva);
    uint8_t *metadata_root = (uint8_t *)base + cli_header->MetaData.VirtualAddress;

    METADATA_HEADER *metadata_header = (METADATA_HEADER *)metadata_root;
    if (metadata_header->Signature != 0x424A5342)
        return 0;

    STREAM_HEADER *stream = (STREAM_HEADER *)((uint8_t *)metadata_header + sizeof(METADATA_HEADER) + metadata_header->VersionLength);
    TABLES_HEADER *tables = 0;

    for (int i = 0; i < metadata_header->Streams; i++)
    {
        const char *stream_name = (const char *)((uint8_t *)stream + sizeof(STREAM_HEADER));
        if (strcmp(stream_name, "#~") == 0)
        {
            tables = (TABLES_HEADER *)(metadata_root + stream->Offset);
            break;
        }
        stream = (STREAM_HEADER *)((uint8_t *)stream + sizeof(STREAM_HEADER) + strlen(stream_name) + 1);
        stream = (STREAM_HEADER *)(((uintptr_t)stream + 3) & ~3);
    }

    if (!tables)
        return 0;

    uint8_t table_idx = (token >> 24) & 0x3F;
    uint32_t row_idx = token & 0x00FFFFFF;
    if (table_idx != 0x06 || row_idx == 0)
        return 0;

    size_t offset = 0;
    uint8_t *m_rows = (uint8_t *)tables + sizeof(TABLES_HEADER);
    for (int row = 0; row < 64; ++row)
    {
        if (!(tables->Valid & (1ULL << row)))
            continue;
        offset += *(uint32_t *)m_rows * table_row_size(row, tables->HeapSizes);
        m_rows += sizeof(uint32_t);
    }

    uint8_t *method_table = m_rows + offset;
    bool string_heap = (tables->HeapSizes & 0x01) != 0;
    bool blob_heap = (tables->HeapSizes & 0x04) != 0;
    size_t method_table_row_size = 4 + 2 + 2 + (string_heap ? 4 : 2) + (blob_heap ? 4 : 2) + 2;
    return (uint32_t *)(method_table + (row_idx - 1) * method_table_row_size);
}
