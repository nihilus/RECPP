#include "IDAUtils.h"
#include "offset.hpp"
#include "frame.hpp"
#include "struct.hpp"


void
asprintf (char *buffer, char *format, ...)
{	
	if (!format) {
        return;
    }

	va_list args; 
	va_start (args,format); 
	int size = _vscprintf (format, args); 
	
	if (size > 0) {
		size++;
        _vsnprintf (buffer, size, format, args);
	}

	va_end (args);
}



int
str_n_pos (char *str, const char *search, int len_str)
{
	int i,  len_string = len_str,
            len_search = strlen (search);
	int count = 0;

	for (i = 0; i < len_string; i++)
	{
		if (str[i] == search[count])
			count++;
		else
			count = 0;

		if (count == len_search)
		{
			return i - len_search + 1;
		}
	}

	return -1;
}

int
str_pos (const char *str, const char *search)
{
	return str_n_pos ((char *) str, search, strlen (str));
}


char *
IDAUtils::getAsciizStr (
    ea_t address,
    char *buffer,
    size_t bufferSize
) {
    char curByte;
    size_t bufferPos = 0;

    while (curByte = get_byte (address)) {
        buffer [bufferPos++] = curByte;
        address++;
        if (bufferPos > bufferSize) {
            msg ("Type name too long, cannot continue.\n");
            return NULL;
        }
    }

    buffer[bufferPos] = '\0';
    return buffer;
}

void 
IDAUtils::DeleteArray (
    int id
) {
    netnode n(id);
    n.kill();
}

nodeidx_t 
IDAUtils::GetArrayId (
        char *name
) {
    char buffer[2048] = {0};
    qsnprintf (buffer, sizeof(buffer), "$ idc_array %s", name);
    netnode n(buffer);
    return n;
}

ea_t 
IDAUtils::DfirstB (
    int to
) {
    return get_first_dref_to (to);
}

ea_t 
IDAUtils::DnextB (
    int to,
    int current
) {
    return get_next_dref_to (to, current);
}

bool 
IDAUtils::MakeDword (
    ea_t address
) {
    return do_data_ex (address, FF_DWRD, 4, BADADDR);
}

void 
IDAUtils::Unknown (
    ea_t address, 
    size_t size
) {
    if (address == BADADDR) {
        return;
    }

    do_unknown_range (address, size, 0);
}

bool
IDAUtils::ForceDword (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return false;
    }

    if (!IDAUtils::MakeDword (address)) {
        IDAUtils::Unknown (address, 4);
        return IDAUtils::MakeDword (address);
    }

    return true;
}

bool
IDAUtils::hasName (
    flags_t flags
) {
    return has_name (flags);
}

uint32
IDAUtils::CreateArray (
    char *name
) {
    char buf[2048] = {0};

    qsnprintf(buf, sizeof(buf), "$ idc_array %s", name);
    netnode n(buf, 0, true);
    return (nodeidx_t)n;
}

bool
IDAUtils::SetArrayLong (
    uint32 id,
    nodeidx_t idx,
    nodeidx_t value
) {
    netnode n(id);
    return n.altset(idx, value);
}

nodeidx_t
IDAUtils::GetFirstIndex (
    long tag,
    int id
) {
    netnode n(id);
    return n.sup1st((char) tag);
}

int
IDAUtils::MakeCode (
    ea_t address
) {
    return ua_code (address);
}

uint32
IDAUtils::Dword (
    ea_t address
) {
    return get_long (address);
}

tid_t
IDAUtils::GetFrame (
    ea_t address
) {
    func_t *f = get_func (address);
    struc_t *frame = get_frame (f);
    return frame->id;
}

size_t
IDAUtils::GetFrameArgsSize (
    ea_t address
) {
    func_t *f = get_func (address);
    area_t range;
    get_frame_part (f, FPC_ARGS, &range);
    return range.size ();
}

size_t
IDAUtils::GetFrameLvarSize (
    ea_t address
) {
    func_t *f = get_func (address);
    area_t range;
    get_frame_part (f, FPC_LVARS, &range);
    return range.size ();
}

size_t
IDAUtils::GetFrameRegsSize (
    ea_t address
) {
    func_t *f = get_func (address);
    area_t range;
    get_frame_part (f, FPC_SAVREGS, &range);
    return range.size ();
}

uval_t
IDAUtils::MakeFrame (
    ea_t address,
    int lvsize,
    int frregs,
    int argsize
) {
    func_t *f = get_func(address);
    set_frame_size(f, lvsize, frregs, argsize);
    return f->frame;
}


size_t
IDAUtils::GetStrucSize (
    tid_t id
) {
    return get_struc_size (id);
}

bool
IDAUtils::DelStrucMember (
    tid_t id,
    int offset
) {
    return del_struc_member (get_struc(id), offset);
}

void
IDAUtils::ForceStrucMember (
    tid_t id, 
    int offset, 
    tid_t sub_id, 
    char *name
) {
    size_t a, i;

    i = IDAUtils::GetStrucSize (sub_id);

    if (IDAUtils::AddStrucMember (id,name,offset,FF_DATA|FF_STRU,sub_id,i) != 0)
    {
        for (a = offset; a < offset + i; a++) {
            IDAUtils::DelStrucMember(id,a);
        }
        
        IDAUtils::AddStrucMember(id,name,offset,FF_DATA|FF_STRU,sub_id,i);
        //SetMemberName(id, offset, name);
    }
}


size_t 
IDAUtils::GetArraySize (
    tid_t id
) {
    tid_t idx;
    size_t count = 0;
  
    for (idx = IDAUtils::GetFirstIndex (AR_LONG, id); idx != -1; idx = IDAUtils::GetNextIndex (AR_LONG, id, idx)) {
        count++;
    }

    return count;
}

void
IDAUtils::doAddrList (
    char *name
) {
    tid_t idx, id;
    ea_t val, ctr, dtr;

    id = IDAUtils::GetArrayId("AddrList");
    ctr = 0; dtr = 0;

    if (name != NULL && id != -1)
    {
        msg ("refcount:%d\n", IDAUtils::GetArraySize (id));
        if (IDAUtils::GetArraySize (id) != 2) {
            return;
        }
        for ( idx = IDAUtils::GetFirstIndex (AR_LONG, id); idx != -1; idx = IDAUtils::GetNextIndex (AR_LONG, id, idx) )
        {
            val = IDAUtils::GetArrayElement(AR_LONG, id, idx);
            
            if (IDAUtils::Byte (val) == 0xE9) {
                val = IDAUtils::getRelJmpTarget(val);
            }

            if ((strncmp (IDAUtils::Name (val), "??1", 3) == 0)) {
                dtr = val;
            }
            else {
                ctr = val;
            }
        }
    }

    if (ctr != 0 && dtr != 0) {
        msg ("  constructor at %a\n", ctr);
        char buffer[2048] = {0};
        IDAUtils::MakeName(ctr, IDAUtils::MakeSpecialName (name, SN_constructor, 0, buffer));
    }

    IDAUtils::DeleteArray (IDAUtils::GetArrayId ("AddrList"));
}


//add (or rename) a stack variable named name at frame offset offset (i.e. bp-based)
//struc_id = structure variable
//if struc_id == -1, then add a dword
void
IDAUtils::CommentStack (
    ea_t start, 
    int offset, 
    char *name, 
    uint32 struc_id
) {
    tid_t id = IDAUtils::GetFrame (start);
    int frameSize = IDAUtils::GetFrameLvarSize (start);
    
    if ((IDAUtils::GetFunctionFlags (start) & FUNC_FRAME) == 0) {
        frameSize = frameSize + IDAUtils::GetFrameRegsSize (start);
    }
    
    frameSize = frameSize + offset;

    // Message("%a: ebp offset = %02Xh\n",start,l);
    if (frameSize < 0) {    
        // Message("growing the frame to locals=%d, regs=4, args=%d.\n",-offset, GetFrameArgsSize(start));
        // We need to grow the locals
        IDAUtils::MakeFrame (start, -offset, IDAUtils::GetFrameRegsSize (start), IDAUtils::GetFrameArgsSize (start));
        frameSize = 0;
    }

    if (struc_id == -1) {
        IDAUtils::ForceDWMember (id, frameSize, name);
    }

    else {
        IDAUtils::ForceStrucMember (id, frameSize, struc_id, name);
    }
}

int
IDAUtils::AddStrucMember (
    long id,
    char *name,
    long offset,
    long flag,
    long type,
    long nbytes
) {
    typeinfo_t mt;
    // Calls an internal function to initialize mt using typeid
    return add_struc_member (get_struc (id), name, offset, flag, &mt, nbytes);
}

void
IDAUtils::SetMemberName (
    long id,
    long member_offset,
    char *name
) {
    set_member_name (get_struc (id), member_offset, name);
}

void
IDAUtils::ForceDWMember (
    tid_t id, 
    int offset, 
    char *name
) {
    if (IDAUtils::AddStrucMember (id, name, offset, FF_DWRD, -1, 4) != 0) {
        IDAUtils::SetMemberName (id, offset, name);
    }
}

ea_t
IDAUtils::getRelCallTarget (
    ea_t address
) {
    char b;

    b = IDAUtils::Byte (address);
  
    if (b == 0xE8)
    {
        b = IDAUtils::Dword (address + 1);
        if (b & 0x80000000) {
            return address + 5 - (~b + 1);
        }
        else {
            return address + 5 + b;
        }
    }
       
    return BADADDR;
}


char *
IDAUtils::MakeSpecialName (
    char *name, 
    uint32 type, 
    uint32 adj,
    char *buffer
) {
    char *basename;

    //.?AUA@@ = typeid(struct A)
    // basename = A@@
    basename = &name[4];

    switch (type) 
    {
        case SN_constructor: {
            //??0A@@QAE@XZ = public: __thiscall A::A(void)
            if (adj == 0) { 
                asprintf (buffer, "??0%sQAE@XZ", basename);
                return buffer;
            }
            else {
                char buffer2[2048] = {0};
                asprintf (buffer, "??0%sW%sAE@XZ", basename, IDAUtils::MangleNumber (adj, buffer2));
                return buffer;
            }
        } break;

        case SN_destructor: {
            //??1A@@QAE@XZ = "public: __thiscall A::~A(void)"
            if (adj == 0) { 
                asprintf (buffer, "??1%sQAE@XZ", basename);
                return buffer;
            }
            else {
                char buffer2[2048] = {0};
                asprintf (buffer, "??1%sW%sAE@XZ", basename, IDAUtils::MangleNumber (adj, buffer2));
                return buffer;
            }
        } break;

        case SN_vdestructor: {
            //??1A@@UAE@XZ = public: virtual __thiscall A::~A(void)
            if (adj == 0) { 
                asprintf (buffer, "??1%sUAE@XZ", basename);
                return buffer;
            }
            else {
                char buffer2[2048] = {0};
                asprintf (buffer, "??1%sW%sAE@XZ", basename, IDAUtils::MangleNumber (adj, buffer2));
                return buffer;
            }
        } break;

        case SN_scalardtr: {
            //??_GA@@UAEPAXI@Z = public: virtual void * __thiscall A::`scalar deleting destructor'(unsigned int)
            if (adj == 0) { 
                asprintf (buffer, "??_G%sUAEPAXI@Z", basename);
                return buffer;
            }
            else {
                char buffer2[2048] = {0};
                asprintf (buffer, "??_G%sW%sAEPAXI@Z", basename, IDAUtils::MangleNumber (adj, buffer2));
                return buffer;
            }
        } break;

        case SN_vectordtr: {
            //.?AUA@@ = typeid(struct A)
            //??_EA@@UAEPAXI@Z = public: virtual void * __thiscall A::`vector deleting destructor'(unsigned int)
            if (adj == 0) { 
                asprintf (buffer, "??_E%sQAEPAXI@Z", basename);
                return buffer;
            }
            else {
                char buffer2[2048] = {0};
                asprintf (buffer, "??_E%sW%sAEPAXI@Z", basename, IDAUtils::MangleNumber (adj, buffer2));
                return buffer;
            }
        } break;

        default: msg ("Wrong special name type\n"); return NULL; break;
    }
    
    return NULL;
}



//check if values match a pattern
bool
IDAUtils::matchBytes (
    ea_t address,
    char *match
) {
    size_t len = strlen(match);

    if (len % 2) { 
        msg ("Bad match string in matchBytes: %s", match);
        return false;
    }

    size_t i = 0;
    char s[3] = {0};
    char s2[3] = {0};

    while (i < len)
    {
        s[0] = match[i];
        s[1] = match[i+1];
        sprintf_s (s2, "%02X", IDAUtils::Byte (address));

        if ((strncmp (s, "??", 2) != 0)
        &&  (strncmp (s2, s, 2) != 0)
        ) {
            // Mismatch
            return false;
        }

        i += 2;
        address++;
    }

    return true;
}


ea_t
IDAUtils::getRelJmpTarget (
    ea_t address
) {
    char b;

    b = IDAUtils::Byte (address);

    if (b == 0xEB) {
        b = IDAUtils::Byte (address + 1);

        if (b & 0x80) {
            return address + 2 - ((~b & 0xFF) + 1);
        }
        else {
            return address + 2 + b;
        }
    }

    else if (b==0xE9) {
        b = IDAUtils::Dword (address + 1);
        if (b & 0x80000000) {
            return address + 5 - (~b + 1);
        }
        else {
            return address + 5 + b;
        }
    }
    else
    return BADADDR;
}

unsigned char
IDAUtils::Byte (
    ea_t address
) {
    return get_full_byte(address);
}

bool
IDAUtils::MakeFunction (
    ea_t start,
    ea_t end
) {
    return add_func (start, end);
}

nodeidx_t
IDAUtils::GetLastIndex (
    long tag,
    int id
) {
    netnode n(id);
    return n.suplast ((char) tag);
}

nodeidx_t
IDAUtils::GetNextIndex (
    long tag,
    int id,
    nodeidx_t idx
) {
    netnode n(id);
    return n.supnxt (idx, (char) tag);
}

ea_t
IDAUtils::GetArrayElement (
    long tag,
    int id,
    nodeidx_t idx
) {
    char buf[2048] = {0};
    netnode n(id);

    if (tag == 'A') {
        return n.altval(idx);
    }

    else if (tag == 'S') {
        n.supstr(idx, buf, sizeof(buf));
    }

    return (ea_t) qstrdup(buf);
}

bool
IDAUtils::AddAddr (
    ea_t address
) {
    nodeidx_t id, idx;
    ea_t val;

    if ((id = IDAUtils::GetArrayId ("AddrList")) == -1) {
        id  = IDAUtils::CreateArray ("AddrList");
        IDAUtils::SetArrayLong (id, 0, address);
        return true;
    }

    for (idx = IDAUtils::GetFirstIndex (AR_LONG, id); idx != -1; idx = IDAUtils::GetNextIndex (AR_LONG, id, idx))
    {
        val = IDAUtils::GetArrayElement (AR_LONG, id, idx);
        if (val == address) {
            return true;
        }

        if (val > address)    // InSort
        {
            for (; idx != -1; idx = IDAUtils::GetNextIndex (AR_LONG, id, idx)) {
                val = IDAUtils::GetArrayElement (AR_LONG, id, idx);
                IDAUtils::SetArrayLong (id, idx, address);
                address = val;
            }
        }
    }

    IDAUtils::SetArrayLong (id, IDAUtils::GetLastIndex (AR_LONG, id) + 1, address);
    return true;
}

char *
IDAUtils::Name (
    ea_t address
) {
    char buf[2048] = {0};
    size_t bufsize = sizeof (buf);
    char *result = get_name (BADADDR, address, buf, bufsize);
    if (!result) {
        return strdup ("");
    }
    return result;
}

ea_t
IDAUtils::PrevFunction (
    ea_t address
) {
    return get_prev_func (address)->startEA;
}

ushort
IDAUtils::GetFunctionFlags (
    ea_t address
) {
    func_t *f = get_func (address);
    if (!f) {
        return 0;
    }

    return f->flags;
}

flags_t
IDAUtils::GetFlags (
    ea_t address
) {
    return getFlags (address);
}

bool
IDAUtils::MakeComm (
    ea_t address,
    char *comment
) {
    return set_cmt (address, comment, false);
}

bool
IDAUtils::DwordCmt (
    ea_t address, 
    char *comment
) {
    if (address == BADADDR || !address) {
        return false;
    }

    IDAUtils::ForceDword (address);
    return IDAUtils::MakeComm (address, comment);
}


char *
IDAUtils::Demangle (
    char *mangledName,
    uint32 disable_mask,
    char *buffer,
    size_t bufferSize
) {
    demangle_name (buffer, bufferSize, mangledName, disable_mask);
    return buffer;
}


char *
IDAUtils::DemangleTIName (
    char *mangledName,
    char *result,
    size_t resultSize
) {
    if (mangledName[0] != '.') {
        return NULL;
    }

    char buffer[2048] = {0};

    asprintf (buffer, "??_R0%s@8", mangledName);
    IDAUtils::Demangle (buffer, 8, result, resultSize);

    char *end = strstr (result, "`RTTI Type Descriptor'");
    if (end == NULL) {
        return NULL;
    }
    else {
        *end = '\0';
        return result;
    }
}

bool
IDAUtils::OpOff (
    ea_t address,
    int n,
    ea_t base
) {
    if (base != BADADDR) {
        return op_offset (address, n, REF_OFF32, BADADDR, base) ? true : false;
    }

    return noType (address, n);
}

bool
IDAUtils::SoftOff (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return false;
    }

    if (!(IDAUtils::ForceDword (address))) {
        msg ("Cannot force dword at %#x\n", address);
        return false;
    }
   
    if (get_long (address) > 0 && get_long (address) <= inf.maxEA) {
        return OpOff (address, 0, 0);
    }

    return false;
}

bool
IDAUtils::StrCmt (
    ea_t address, 
    char *comment
) {
    if (address == BADADDR || !address) {
        return false;
    }

    IDAUtils::MakeUnkn (address, 0);
    int save_str = IDAUtils::GetLongPrm (INF_STRTYPE);
    IDAUtils::SetLongPrm (INF_STRTYPE,0);
    IDAUtils::MakeStr (address, BADADDR);
    IDAUtils::MakeName (address, "");
    IDAUtils::MakeComm (address, comment);
    IDAUtils::SetLongPrm (INF_STRTYPE,save_str);

    return true;
}

bool
IDAUtils::OffCmt (
    ea_t address, 
    char *comment
) {
    if (address == BADADDR || !address) {
        return false;
    }

    if (!(IDAUtils::SoftOff (address))) {
        msg ("Cannot softOff at %#x\n", address);
        return false;
    }

    return IDAUtils::MakeComm (address, comment);
}


bool
IDAUtils::MakeName (
    ea_t address,
    char *name
) {
    return set_name (address, name, SN_CHECK);
}

bool
IDAUtils::MakeArray (
    ea_t address,
    size_t nItems
) {
    typeinfo_t ti;
    flags_t f = get_flags_novalue (address);
    get_typeinfo (address, 0, f, &ti);
    asize_t sz = get_data_elsize (address, f, &ti);
    return do_data_ex (address, f, sz * nItems, ti.tid);
}

bool
IDAUtils::DwordArrayCmt (
    ea_t address, 
    size_t n, 
    char *comment
) {
    if (address == BADADDR || !address) {
        return false;
    }
    
    IDAUtils::Unknown (address, 4 * n);

    if (!IDAUtils::ForceDword (address)
    ||  !IDAUtils::MakeArray (address, n)
    ||  !IDAUtils::MakeComm (address, comment)) {
        return false;
    }

    return true;
}

void 
IDAUtils::MakeUnkn (
    ea_t address,
    int flags
) {
    return do_unknown(address, flags);
}

int
IDAUtils::GetLongPrm (
    long int offset
) {
    if (offset <= 188) {
        return *(int*)(offset + (char*)&inf);
    }

    return -1;
}

bool
IDAUtils::SetLongPrm (
    long int offset,
    long int value
) {
    if (offset >= 13 && offset <= 188) {
        *(int*)(offset + (char*)&inf) = value;
    }

    return true;
}

bool
IDAUtils::MakeStr (
    long address, 
    long endAddress
) {
    int len = endAddress == -1 ? 0 : endAddress - address;
    return make_ascii_string (address, len, ASCSTR_C);
}

char *
IDAUtils::MangleNumber (
    int number,
    char *buffer
) {
    //
    // 0 = A@
    // X = X-1 (1<=X<=10)
    // -X = ?(X-1)
    // 0x0..0xF = 'A'..'P'

    buffer[0] = '\0';
    int sign = 0;

    if (number < 0) {
        sign = 1;
        number = -number;
    }

    if (number == 0) {
        return strdup ("A@");
    }

    else if (number <= 10) {
        asprintf (buffer, "%s%d", sign ? "?" : "", number - 1);
        return buffer;
    }

    else {
        while (number > 0)
        {
            asprintf (buffer, "%c%s", 'A' + (number % 16), buffer);
            number = number / 16;
        }
        asprintf (buffer, "%s%s@", sign ? "?" : "", buffer);
        return buffer;
    }
}