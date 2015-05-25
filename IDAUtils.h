/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/
#pragma once

// ---------- Includes ------------
#include "RECPP.h"

// ---------- Defines -------------
#ifndef INF_STRTYPE
#define INF_STRTYPE     154             // long;    current ascii string type
#endif
#ifndef AR_LONG
#define AR_LONG 'A'     // array of longs
#endif
#ifndef AR_STR
#define AR_STR  'S'     // array of strings
#endif

#define SN_constructor 1
#define SN_destructor  2
#define SN_vdestructor 3
#define SN_scalardtr   4
#define SN_vectordtr   5

// ------ Class declaration -------
class IDAUtils {
    public:

    /*
    * @brief : Get a string terminated with a zero at \address
    * @param address : The address of the string
    * @return The string, or NULL if an error occured
    */
    static char *
    getAsciizStr (
        ea_t address,
        char *buffer,
        size_t bufferSize
    );

    /*
    * @brief :
    */
    static bool
    DwordCmt (
        ea_t x, 
        char *cmt
    );

    bool
    IDAUtils::MakeNameForce (
        ea_t address,
        char *name
    );

    /*
    * @brief : Make dword, undefine as needed
    * @param address : The start address
    */
    static bool
    ForceDword (
        ea_t address
    );

    /*
    * @brief : Mark the ea as unknown for a length of length, but don't propagate.
    * @param address : The start address
    * @param size : The size of the unknown data
    */
    static void 
    Unknown (
        ea_t address, 
        size_t size
    );

    
    /*
    * @brief : Make an address as a DWORD
    * @param address : The start address
    */
    static bool 
    MakeDword (
        ea_t address
    );
    
    /*
    * @brief : Set a comment at \address
    * @param address : The start address
    * @param comment : The comment
    */
    static bool
    MakeComm (
        ea_t x,
        char *comment
    );

    /*
    * @brief :
    */
    static bool
    OffCmt (
        ea_t address, 
        char *comment
    );

    /*
    * @brief :
    */
    static bool
    SoftOff (
        ea_t address
    );
    
    /*
    * @brief :
    */
    static bool
    OpOff (
        ea_t address,
        int n,
        ea_t base
    );
    
    /*
    * @brief :
    */
    static char *
    IDAUtils::Demangle (
        char *mangledName,
        uint32 disable_mask,
        char *buffer,
        size_t bufferSize
    );
    
    /*
    * @brief : Demangle names like .?AVxxx, .PAD, .H etc
    */
    static char *
    DemangleTIName (
        char *mangledName,
        char *buffer,
        size_t resultSize
    );
    
    /*
    * @brief : Give a name to a given address
    */
    static bool
    MakeName (
        ea_t address,
        char *name
    );
    
    /*
    * @brief : Make an array at the given address
    */
    static bool
    MakeArray (
        ea_t address,
        size_t nItems
    );
    
    /*
    * @brief : 
    */
    static bool
    DwordArrayCmt (
        ea_t address, 
        size_t n, 
        char *comment
    );

    /*
    * @brief : 
    */
    static void 
    MakeUnkn (
        ea_t address,
        int flags
    );
    
    /*
    * @brief : 
    */
    static bool
    StrCmt (
        ea_t address, 
        char *comment
    );

    /*
    * @brief : 
    */
    static int
    GetLongPrm (
        long int offset
    );
    
    static bool
    MakeStr (
        long address, 
        long endAddress
    );

    /*
    * @brief : 
    */
    static bool
    SetLongPrm (
        long int offset,
        long int value
    );

    /*
    * @brief : 
    */
    static char *
    MangleNumber (
        int number,
        char *buffer,
        size_t bufferSize
    );
    
    /*
    * @brief : 
    */
    static void 
    IDAUtils::DeleteArray (
        int id
    );

    /*
    * @brief : 
    */
    static nodeidx_t 
    IDAUtils::GetArrayId (
        char *name
    );

    /*
    * @brief : 
    */
    static ea_t 
    IDAUtils::DfirstB (
        int to
    );

    /*
    * @brief : 
    */
    static ea_t 
    IDAUtils::DnextB (
        int to,
        int current
    );
    
    /*
    * @brief : 
    */
    static bool
    IDAUtils::hasName (
        flags_t flags
    );

    /*
    * @brief : 
    */
    static flags_t
    IDAUtils::GetFlags (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static ushort
    IDAUtils::GetFunctionFlags (
        ea_t address
    );

    /*
    * @brief : 
    */
    static ea_t
    IDAUtils::PrevFunction (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static char *
    IDAUtils::Name (
        ea_t address,
        char *buffer,
        size_t bufferSize
    );

    /*
    * @brief : 
    */
    static uint32
    IDAUtils::CreateArray (
        char *name
    );

    /*
    * @brief : 
    */
    static bool
    IDAUtils::SetArrayLong (
        uint32 id,
        nodeidx_t idx,
        nodeidx_t value
    );

    /*
    * @brief : 
    */
    static nodeidx_t
    IDAUtils::GetFirstIndex (
        long tag,
        int id
    );

    /*
    * @brief : 
    */
    static nodeidx_t
    IDAUtils::GetNextIndex (
        long tag,
        int id,
        nodeidx_t idx
    );
    
    /*
    * @brief : 
    */
    static nodeidx_t
    IDAUtils::GetLastIndex (
        long tag,
        int id
    );
    
    /*
    * @brief : 
    */
    static ea_t
    IDAUtils::GetArrayElementA (
        int id,
        nodeidx_t idx
    );

    /*
    * @brief : 
    */
    static bool
    IDAUtils::AddAddr (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static int
    IDAUtils::MakeCode (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static bool
    IDAUtils::MakeFunction (
        ea_t start,
        ea_t end
    );
    
    /*
    * @brief : 
    */
    static unsigned char
    IDAUtils::Byte (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static uint32
    IDAUtils::Dword (
        ea_t address
    );

    
    /*
    * @brief : 
    */
    static ea_t
    IDAUtils::getRelJmpTarget (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static bool
    IDAUtils::matchBytes (
        ea_t address,
        char *match
    );
    
    /*
    * @brief : 
    */
    static char *
    IDAUtils::MakeSpecialName (
        char *name, 
        uint32 type, 
        uint32 adj,
        char *buffer,
        size_t bufferSize
    );
    
    /*
    * @brief : 
    */
    static ea_t
    IDAUtils::getRelCallTarget (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static tid_t
    IDAUtils::GetFrame (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static size_t
    IDAUtils::GetFrameLvarSize (
        ea_t address
    );
    
    /*
    * @brief : 
    */
    static size_t
    IDAUtils::GetFrameRegsSize (
        ea_t address
    );

    /*
    * @brief : 
    */
    static size_t
    IDAUtils::GetFrameArgsSize (
        ea_t address
    );
    
    /*
    * @brief : Add (or rename) a stack variable named name at frame offset offset (i.e. bp-based)
    */
    static void
    IDAUtils::CommentStack (
        ea_t start, 
        int offset, 
        char *name, 
        uint32 struc_id
    );
    
    /*
    * @brief : 
    */
    static uval_t
    IDAUtils::MakeFrame (
        ea_t address,
        int lvsize,
        int frregs,
        int argsize
    );

    /*
    * @brief : 
    */
    static int
    IDAUtils::AddStrucMember (
        long id,
        char *name,
        long offset,
        long flag,
        long type,
        long nbytes
    );

    /*
    * @brief :
    */
    static void
    IDAUtils::SetMemberName (
        long id,
        long member_offset,
        char *name
    );

    /*
    * @brief :
    */
    static void
    IDAUtils::ForceDWMember (
        tid_t id, 
        int offset, 
        char *name
    );
    
    /*
    * @brief :
    */
    static size_t
    IDAUtils::GetStrucSize (
        tid_t id
    );
    
    /*
    * @brief :
    */
    static bool
    IDAUtils::DelStrucMember (
        tid_t id,
        int offset
    );
    
    /*
    * @brief :
    */
    static void
    IDAUtils::ForceStrucMember (
        tid_t id, 
        int offset, 
        tid_t sub_id, 
        char *name
    );
    
    /*
    * @brief :
    */
    static size_t 
    IDAUtils::GetArraySize (
        tid_t id
    );
    
    /*
    * @brief :
    */
    static void
    IDAUtils::doAddrList (
        char *name
    );
};

int
str_pos (const char *str, const char *search);