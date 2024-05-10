#include "clrhost.h"

#import "mscorlib.tlb" auto_rename, raw_interfaces_only
using namespace mscorlib;

enum class ClassMethodType : int32_t
{
    Load = 0,
    Replay = 1,
    Score = 2,
    CheckFlashlight = 3,
    UpdateFlashlight = 4,
    CheckTime = 5,
    UpdateVariables = 6,
    UpdateMods = 7
};

struct ClassMethod
{
    UINT c_name_length;
    TypeAttributes ca;
    UINT m_name_length;
    MethodAttributes ma;
    int mpc;
    int mpnl[3];
    int mptl[3];
    _Type **mrt;
    ClassMethodType t;
};

std::vector<std::future<void>> prepare_method_tasks;
int prepared_methods_count = -1;
_AssemblyPtr mscorlib_assembly = 0;
_AssemblyPtr osu_assembly = 0;
_TypePtr system_void_type = 0;
_TypePtr system_double_type = 0;

ClassMethod classmethods[] = {
    {
        .c_name_length = sizeof("#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class),
        .m_name_length = sizeof("#=zanE7Qv3XmjW5") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Private | MethodAttributes_HideBySig | MethodAttributes_ReuseSlot),
        .mpc = 0,
        .mpnl = {0, 0, 0},
        .mptl = {0, 0, 0},
        .mrt = &system_void_type,
        .t = ClassMethodType::Load,
    },
    {
        .c_name_length = sizeof("#=zdKJOjAyV$ER12ZW5$R$q$GYWyVHooHal7eJTMqI=") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class),
        .m_name_length = sizeof("#=zz6ecxM6_4e4c6Im$hA==") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Family | MethodAttributes_Virtual | MethodAttributes_HideBySig |
                                 MethodAttributes_VtableLayoutMask | MethodAttributes_ReuseSlot | MethodAttributes_NewSlot),
        .mpc = 0,
        .mpnl = {0, 0, 0},
        .mptl = {0, 0, 0},
        .mrt = &system_void_type,
        .t = ClassMethodType::Replay,
    },
    {
        .c_name_length = sizeof("#=zZxV46FKXz8HxVteOcXctYSVtWa6VKIH_YNsHAm8=") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class | TypeAttributes_Abstract | TypeAttributes_Sealed),
        .m_name_length = sizeof("#=zNFUNmEy9GMdn") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Private | MethodAttributes_FamANDAssem | MethodAttributes_Assembly |
                                 MethodAttributes_Static | MethodAttributes_HideBySig | MethodAttributes_ReuseSlot),
        .mpc = 3,
        .mpnl = { 11, 11, 15 },
        .mptl = { 15, 20, 47 },
        .mrt = &system_double_type,
        .t = ClassMethodType::Score,
    },
    {
        .c_name_length = sizeof("#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class),
        .m_name_length = sizeof("#=z8sU_nCrclRWZEfIsFhlq2S4=") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Private | MethodAttributes_HideBySig | MethodAttributes_ReuseSlot),
        .mpc = 0,
        .mpnl = {0, 0, 0},
        .mptl = {0, 0, 0},
        .mrt = &system_void_type,
        .t = ClassMethodType::CheckFlashlight,
    },
    {
        .c_name_length = sizeof("#=z3onHVu8ArxqQRwlNg7LafPTccV82AY6DfUBBuhGZ$HyZ0LAnfQ==") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class),
        .m_name_length = sizeof("#=zn$KM8OPB3VD3EEQhXA==") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Private | MethodAttributes_FamANDAssem | MethodAttributes_Assembly |
                                 MethodAttributes_Virtual | MethodAttributes_HideBySig | MethodAttributes_CheckAccessOnOverride | MethodAttributes_ReuseSlot),
        .mpc = 0,
        .mpnl = {0, 0, 0},
        .mptl = {0, 0, 0},
        .mrt = &system_void_type,
        .t = ClassMethodType::UpdateFlashlight,
    },
    {
        .c_name_length = sizeof("#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class),
        .m_name_length = sizeof("#=zr__5T0o=") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_FamANDAssem | MethodAttributes_Family | MethodAttributes_Public |
                                 MethodAttributes_Virtual | MethodAttributes_HideBySig | MethodAttributes_ReuseSlot),
        .mpc = 0,
        .mpnl = {0, 0, 0},
        .mptl = {0, 0, 0},
        .mrt = &system_void_type,
        .t = ClassMethodType::CheckTime,
    },
    {
        .c_name_length = sizeof("#=zE0VDZfwJEH3z6D3XGqmTkDRFkwfG") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class | TypeAttributes_Abstract),
        .m_name_length = sizeof("") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Private | MethodAttributes_FamANDAssem | MethodAttributes_Assembly |
                                 MethodAttributes_Virtual | MethodAttributes_HideBySig | MethodAttributes_CheckAccessOnOverride |
                                 MethodAttributes_VtableLayoutMask | MethodAttributes_ReuseSlot | MethodAttributes_NewSlot),
        .mpc = 3,
        .mpnl = { 23, 23, 27 },
        .mptl = { 14, 14, 14 },
        .mrt = &system_void_type,
        .t = ClassMethodType::UpdateVariables,
    },
    {
        .c_name_length = sizeof("#=zNDts0g6JXE$Y_YpheSvbgzzhzpoQ") - 1,
        .ca = (TypeAttributes)(TypeAttributes_NotPublic | TypeAttributes_AutoLayout | TypeAttributes_Class | TypeAttributes_Sealed),
        .m_name_length = sizeof("#=zO5qmbpU=") - 1,
        .ma = (MethodAttributes)(MethodAttributes_PrivateScope | MethodAttributes_Private | MethodAttributes_HideBySig | MethodAttributes_ReuseSlot),
        .mpc = 2,
        .mpnl = { 11, 11 },
        .mptl = { 6, 9 },
        .mrt = &system_void_type,
        .t = ClassMethodType::UpdateMods,
    },
};

static inline std::string get_utf8(const std::wstring &wstr)
{
    if (wstr.empty()) return std::string();
    int sz = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, 0, 0, 0, 0);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &res[0], sz, 0, 0);
    return res;
}

template<typename... T>
static inline SAFEARRAY* get_params(T... args)
{
    if constexpr (sizeof...(args))
    {
        SAFEARRAY* params = SafeArrayCreateVector(VT_VARIANT, 0, sizeof...(args));
        LONG i = 0;
        for (const auto arg : {args...})
        {
            SafeArrayPutElement(params, &i, arg);
            ++i;
        }
        return params;
    }
    else
        return SafeArrayCreateVector(VT_EMPTY, 0, 0);
}

template<typename... T>
static inline SAFEARRAY* get_types(_AssemblyPtr pAssembly, T... args)
{
    if (pAssembly == 0)
    {
        FR_ERROR("get_types failed, pAssembly is null");
        return 0;
    }

    SAFEARRAY* types = SafeArrayCreateVector(VT_UNKNOWN, 0, sizeof...(args));
    LONG i = 0;
    for (const wchar_t *type_s : {args...})
    {
        _TypePtr type_ptr = 0;
        BSTR type_name_b = SysAllocString(type_s);
        HRESULT hr = pAssembly->GetType_2(type_name_b, &type_ptr);
        SysFreeString(type_name_b);
        if (!type_ptr)
        {
            std::string type_name_s = get_utf8(type_s);
            FR_ERROR("GetType (%s, 0x%X)", type_name_s.c_str(), hr);
            SafeArrayDestroy(types);
            return 0;
        }
        SafeArrayPutElement(types, &i, type_ptr);
        ++i;
    }
    return types;
}

static inline bool variant_ok(VARIANT variant)
{
    return V_VT(&variant) != VT_EMPTY;
}

static inline bool assemblies_loaded(const char *method)
{
    if (osu_assembly == 0)
        FR_ERROR("%s failed, osu! assembly is null", method);
    if (mscorlib_assembly == 0)
        FR_ERROR("%s failed, mscorlib assembly is null", method);
    return osu_assembly && mscorlib_assembly;
}

static inline bool match_class_name_length(_TypePtr c)
{
    BSTR c_name = 0;
    HRESULT hr = c->get_name(&c_name);
    if (!c_name)
    {
        FR_ERROR("get_name (0x%X)", hr);
        return true;
    }
    UINT c_name_length = SysStringLen(c_name);
    SysFreeString(c_name);
    for (const auto &cm : classmethods)
        if (c_name_length == cm.c_name_length)
            return true;
    return false;
}

static inline bool match_method_name_length(_MethodInfoPtr m, ClassMethod cm)
{
    BSTR m_name = 0;
    HRESULT hr = m->get_name(&m_name);
    if (!m_name)
    {
        FR_ERROR("get_name (0x%X)", hr);
        return true;
    }
    UINT m_name_length = SysStringLen(m_name);
    SysFreeString(m_name);
    if (m_name_length == cm.m_name_length)
        return true;
    return false;
}

// NOTE(Ciremun): Compare class and method attributes we are looping with our predefined classes and methods
static bool verify_classmethod(_TypePtr c, _MethodInfoPtr m, ClassMethod cm)
{
    enum TypeAttributes ca;
    HRESULT hr = c->get_Attributes(&ca);
    if (FAILED(hr))
        FR_ERROR("get_Attributes (0x%X)", hr);
    else
    {
        if ((ca & TypeAttributes_Abstract) != (cm.ca & TypeAttributes_Abstract)) return false;
        if ((ca & TypeAttributes_Sealed) != (cm.ca & TypeAttributes_Sealed))     return false;

        TypeAttributes v = (TypeAttributes)(ca & TypeAttributes_VisibilityMask);
        if ((v & TypeAttributes_NotPublic) != (cm.ca & TypeAttributes_NotPublic))                 return false;
        if ((v & TypeAttributes_Public) != (cm.ca & TypeAttributes_Public))                       return false;
        if ((v & TypeAttributes_NestedPublic) != (cm.ca & TypeAttributes_NestedPublic))           return false;
        if ((v & TypeAttributes_NestedPrivate) != (cm.ca & TypeAttributes_NestedPrivate))         return false;
        if ((v & TypeAttributes_NestedFamANDAssem) != (cm.ca & TypeAttributes_NestedFamANDAssem)) return false;
        if ((v & TypeAttributes_NestedAssembly) != (cm.ca & TypeAttributes_NestedAssembly))       return false;
        if ((v & TypeAttributes_NestedFamily) != (cm.ca & TypeAttributes_NestedFamily))           return false;
        if ((v & TypeAttributes_NestedFamORAssem) != (cm.ca & TypeAttributes_NestedFamORAssem))   return false;

        TypeAttributes l = (TypeAttributes)(ca & TypeAttributes_LayoutMask);
        if ((l & TypeAttributes_AutoLayout) != (cm.ca & TypeAttributes_AutoLayout))             return false;
        if ((l & TypeAttributes_SequentialLayout) != (cm.ca & TypeAttributes_SequentialLayout)) return false;
        if ((l & TypeAttributes_ExplicitLayout) != (cm.ca & TypeAttributes_ExplicitLayout))     return false;

        TypeAttributes cs = (TypeAttributes)(ca & TypeAttributes_ClassSemanticsMask);
        if ((cs & TypeAttributes_Class) != (cm.ca & TypeAttributes_Class))         return false;
        if ((cs & TypeAttributes_Interface) != (cm.ca & TypeAttributes_Interface)) return false;
    }

    enum MethodAttributes ma;
    hr = m->get_Attributes(&ma);
    if (FAILED(hr))
        FR_ERROR("get_Attributes (0x%X)", hr);
    else if (ma != cm.ma) return false;

    if (*cm.mrt)
    {
        _TypePtr m_return_type = 0;
        hr = m->get_returnType(&m_return_type);
        if (!m_return_type)
            FR_ERROR("get_returnType (0x%X)", hr);
        VARIANT_BOOL return_types_equal = VARIANT_TRUE;
        hr = m_return_type->Equals_2(*cm.mrt, &return_types_equal);
        if (FAILED(hr))
            FR_ERROR("Equals_2 (0x%X)", hr);
        else if (return_types_equal == VARIANT_FALSE) return false;
    }

    SAFEARRAY *p = 0;
    hr = m->GetParameters(&p);
    if (!p)
        FR_ERROR("GetParameters (0x%X)", hr);
    else
    {
        LONG lcnt2 = 0;
        LONG ucnt2 = 0;
        hr = SafeArrayGetLBound(p, 1, &lcnt2);
        if (FAILED(hr))
        {
            FR_ERROR("SafeArrayGetLBound (0x%X)", hr);
            SafeArrayDestroy(p);
            return true;
        }

        hr = SafeArrayGetUBound(p, 1, &ucnt2);
        if (FAILED(hr))
        {
            FR_ERROR("SafeArrayGetUBound (0x%X)", hr);
            SafeArrayDestroy(p);
            return true;
        }
        SafeArrayDestroy(p);

        LONG p_count = ucnt2 - lcnt2 + 1;
        if (p_count != cm.mpc) return false;
    }

    return true;
}

bool prepare_methods()
{
    if (!assemblies_loaded("prepare_methods"))
        return false;

    // NOTE(Ciremun): returns zero on race-condition
    double s = ImGui::GetTime();

    BSTR system_void_b = SysAllocString(L"System.Void");
    HRESULT hr = mscorlib_assembly->GetType_2(system_void_b, &system_void_type);
    SysFreeString(system_void_b);
    if (!system_void_type)
        FR_ERROR("GetType_2 (0x%X)", hr);

    BSTR system_double_b = SysAllocString(L"System.Double");
    hr = mscorlib_assembly->GetType_2(system_double_b, &system_double_type);
    SysFreeString(system_double_b);
    if (!system_double_type)
        FR_ERROR("GetType_2 (0x%X)", hr);

    _TypePtr runtime_helpers_type = 0;
    BSTR runtime_helpers_b = SysAllocString(L"System.Runtime.CompilerServices.RuntimeHelpers");
    hr = mscorlib_assembly->GetType_2(runtime_helpers_b, &runtime_helpers_type);
    SysFreeString(runtime_helpers_b);
    if (!runtime_helpers_type)
    {
        FR_ERROR("GetType_2 (0x%X)", hr);
        return false;
    }

    SAFEARRAY *types = get_types(mscorlib_assembly, L"System.RuntimeMethodHandle");
    if (!types)
        return false;

    _MethodInfoPtr prepare_method = 0;
    BSTR prepare_method_b = SysAllocString(L"PrepareMethod");
    hr = runtime_helpers_type->GetMethod_5(prepare_method_b, types, &prepare_method);
    SysFreeString(prepare_method_b);
    SafeArrayDestroy(types);
    if (!prepare_method)
    {
        FR_ERROR("GetMethod_5 (0x%X)", hr);
        return false;
    }

    SAFEARRAY *classes = 0;
    hr = osu_assembly->GetTypes(&classes);
    if (!classes)
    {
        FR_ERROR("GetTypes (0x%X)", hr);
        return false;
    }

    LONG lcnt = 0;
    LONG ucnt = 0;
    hr = SafeArrayGetLBound(classes, 1, &lcnt);
    if (FAILED(hr))
    {
        FR_ERROR("SafeArrayGetLBound (0x%X)", hr);
        SafeArrayDestroy(classes);
        return false;
    }

    hr = SafeArrayGetUBound(classes, 1, &ucnt);
    if (FAILED(hr))
    {
        FR_ERROR("SafeArrayGetUBound (0x%X)", hr);
        SafeArrayDestroy(classes);
        return false;
    }
    LONG classes_count = ucnt - lcnt + 1;

    SAFEARRAY* method_handle_args = SafeArrayCreateVector(VT_EMPTY, 0, 0);

    _TypePtr method_info_cls = 0;
    BSTR method_info_cls_b = SysAllocString(L"System.Reflection.MethodInfo");
    mscorlib_assembly->GetType_2(method_info_cls_b, &method_info_cls);
    SysFreeString(method_info_cls_b);

    _PropertyInfoPtr method_handle_prop = 0;
    BSTR method_handle_b = SysAllocString(L"MethodHandle");
    method_info_cls->GetProperty(method_handle_b, (BindingFlags)(BindingFlags_Instance | BindingFlags_Public), &method_handle_prop);
    SysFreeString(method_handle_b);

    prepare_method_tasks.clear();
    prepare_method_tasks.reserve(70);
    for (LONG i = 0; i < classes_count; ++i)
    {
        _TypePtr class_ = 0;
        hr = SafeArrayGetElement(classes, &i, (void *)&class_);
        if (!class_)
        {
            FR_ERROR("SafeArrayGetElement (%ld, 0x%X)", i, hr);
            continue;
        }

        if (!match_class_name_length(class_))
            continue;

        SAFEARRAY *methods = 0;
        hr = class_->GetMethods((BindingFlags)(BindingFlags_DeclaredOnly | BindingFlags_NonPublic | BindingFlags_Public |
                                               BindingFlags_Instance | BindingFlags_Static), &methods);
        if (!methods)
        {
            FR_ERROR("GetMethods (0x%X)", hr);
            continue;
        }

        LONG lcnt2 = 0;
        LONG ucnt2 = 0;
        hr = SafeArrayGetLBound(methods, 1, &lcnt2);
        if (FAILED(hr))
        {
            FR_ERROR("SafeArrayGetLBound (0x%X)", hr);
            SafeArrayDestroy(methods);
            continue;
        }

        hr = SafeArrayGetUBound(methods, 1, &ucnt2);
        if (FAILED(hr))
        {
            FR_ERROR("SafeArrayGetUBound (0x%X)", hr);
            SafeArrayDestroy(methods);
            continue;
        }

        LONG methods_count = ucnt2 - lcnt2 + 1;

        for (LONG j = 0; j < methods_count; ++j)
        {
            _MethodInfoPtr method = 0;
            hr = SafeArrayGetElement(methods, &j, (void *)&method);
            if (!method)
            {
                FR_ERROR("SafeArrayGetElement (%ld, 0x%X)", j, hr);
                continue;
            }

            VARIANT method_handle_ptr;
            VariantInit(&method_handle_ptr);
            V_VT(&method_handle_ptr) = VT_UNKNOWN;
            V_UNKNOWN(&method_handle_ptr) = method;

            VARIANT method_handle_value;
            VariantInit(&method_handle_value);
            method_handle_prop->GetValue(method_handle_ptr, method_handle_args, &method_handle_value);
            for (const auto& cm : classmethods)
            {
                if (!match_method_name_length(method, cm) && cm.t != ClassMethodType::UpdateVariables)
                    continue;

                if (!verify_classmethod(class_, method, cm))
                    continue;

                const auto prepare = [](_MethodInfoPtr prepare_method, VARIANT method_handle_value) {
                    SAFEARRAY *params = get_params(&method_handle_value);
                    HRESULT hr = prepare_method->Invoke_3(method_handle_value, params, &method_handle_value);
                    SafeArrayDestroy(params);
                    if (FAILED(hr))
                        FR_ERROR("Invoke (0x%X)", hr);
                    else
                        ++prepared_methods_count;
                };
                prepare_method_tasks.push_back(
                    std::async(std::launch::async, prepare,
                               prepare_method, method_handle_value));
                break;
            }
        }
        SafeArrayDestroy(methods);
    }
    SafeArrayDestroy(classes);
    SafeArrayDestroy(method_handle_args);

    for (const auto &task : prepare_method_tasks)
        task.wait_for(std::chrono::milliseconds(10 * 1000) / prepare_method_tasks.size());

    FR_INFO("Preparing Methods Took: %lfs", ImGui::GetTime() - s);
    FR_INFO("Prepared Methods: %d", prepared_methods_count);
    return true;
}

intptr_t get_set_presence_ptr()
{
    if (!assemblies_loaded("get_set_presence_ptr"))
        return 0;

    const wchar_t *type_name = L"DiscordRPC.DiscordRpcClient";
    const wchar_t *method_name = L"SetPresence";

    std::string type_name_s = get_utf8(type_name);
    std::string method_name_s = get_utf8(method_name);

    _TypePtr type_ptr = 0;
    BSTR type_name_b = SysAllocString(type_name);
    HRESULT hr = osu_assembly->GetType_2(type_name_b, &type_ptr);
    SysFreeString(type_name_b);
    if (!type_ptr)
    {
        FR_ERROR("%s::%s", type_name_s.c_str(), method_name_s.c_str());
        FR_ERROR("GetType (0x%X)", hr);
        return 0;
    }

    _MethodInfoPtr method_ptr = 0;
    BSTR method_name_b = SysAllocString(method_name);
    hr = type_ptr->GetMethod_6(method_name_b, &method_ptr);
    SysFreeString(method_name_b);
    if (!method_ptr)
    {
        FR_ERROR("%s::%s", type_name_s.c_str(), method_name_s.c_str());
        FR_ERROR("GetMethod (0x%X)", hr);
        return 0;
    }

    _TypePtr method_info_cls = 0;
    BSTR method_info_cls_b = SysAllocString(L"System.Reflection.MethodInfo");
    mscorlib_assembly->GetType_2(method_info_cls_b, &method_info_cls);
    SysFreeString(method_info_cls_b);

    _PropertyInfoPtr method_handle_prop = 0;
    BSTR method_handle_b = SysAllocString(L"MethodHandle");
    method_info_cls->GetProperty(method_handle_b, (BindingFlags)(BindingFlags_Instance | BindingFlags_Public), &method_handle_prop);
    SysFreeString(method_handle_b);

    VARIANT method_handle_ptr;
    VariantInit(&method_handle_ptr);
    V_VT(&method_handle_ptr) = VT_UNKNOWN;
    V_UNKNOWN(&method_handle_ptr) = method_ptr;

    SAFEARRAY* method_handle_args = SafeArrayCreateVector(VT_EMPTY, 0, 0);
    VARIANT method_handle_value;
    VariantInit(&method_handle_value);
    method_handle_prop->GetValue(method_handle_ptr, method_handle_args, &method_handle_value);
    SafeArrayDestroy(method_handle_args);

    _TypePtr rt_method_handle_type = 0;
    BSTR rt_method_handle_b = SysAllocString(L"System.RuntimeMethodHandle");
    mscorlib_assembly->GetType_2(rt_method_handle_b, &rt_method_handle_type);
    SysFreeString(rt_method_handle_b);

    _MethodInfoPtr get_func_ptr_method_info = 0;
    BSTR get_func_ptr_b = SysAllocString(L"GetFunctionPointer");
    rt_method_handle_type->GetMethod_2(get_func_ptr_b, (BindingFlags)(BindingFlags_Public | BindingFlags_Instance), &get_func_ptr_method_info);
    SysFreeString(get_func_ptr_b);

    SAFEARRAY* get_func_ptr_args = SafeArrayCreateVector(VT_EMPTY, 0, 0);
    get_func_ptr_method_info->Invoke_3(method_handle_value, get_func_ptr_args, &method_handle_ptr);
    SafeArrayDestroy(get_func_ptr_args);

    FR_INFO("[+] %s::%s", type_name_s.c_str(), method_name_s.c_str());
    return variant_ok(method_handle_ptr) ? V_INT(&method_handle_ptr) : 0;
}

void free_managed_string(intptr_t gc_handle)
{
    _TypePtr gchandle_type = 0;
    BSTR gchandle_b = SysAllocString(L"System.Runtime.InteropServices.GCHandle");
    HRESULT hr = mscorlib_assembly->GetType_2(gchandle_b, &gchandle_type);
    SysFreeString(gchandle_b);
    if (!gchandle_type)
    {
        FR_ERROR("GetType (0x%X)", hr);
        return;
    }

    _MethodInfoPtr free_method = 0;
    BSTR free_b = SysAllocString(L"Free");
    hr = gchandle_type->GetMethod_6(free_b, &free_method);
    SysFreeString(free_b);
    if (!free_method)
    {
        FR_ERROR("GetMethod(\"Free\") (0x%X)", hr);
        return;
    }

    VARIANT gc_handle_v = *(VARIANT *)gc_handle;
    SAFEARRAY* params = get_params();
    hr = free_method->Invoke_3(gc_handle_v, params, &gc_handle_v);
    free((VARIANT *)gc_handle);
    SafeArrayDestroy(params);
    if (FAILED(hr))
    {
        FR_ERROR("Invoke(\"Free\") (0x%X)", hr);
        return;
    }
}

intptr_t allocate_managed_string(const wchar_t *str, intptr_t *gc_handle)
{
    _TypePtr gchandle_type = 0;
    BSTR gchandle_b = SysAllocString(L"System.Runtime.InteropServices.GCHandle");
    HRESULT hr = mscorlib_assembly->GetType_2(gchandle_b, &gchandle_type);
    SysFreeString(gchandle_b);
    if (!gchandle_type)
    {
        FR_ERROR("GetType (0x%X)", hr);
        return 0;
    }

    SAFEARRAY *types = get_types(mscorlib_assembly, L"System.Object", L"System.Runtime.InteropServices.GCHandleType");
    if (!types)
        return 0;

    _MethodInfoPtr alloc_method = 0;
    BSTR alloc_b = SysAllocString(L"Alloc");
    hr = gchandle_type->GetMethod_5(alloc_b, types, &alloc_method);
    SysFreeString(alloc_b);
    SafeArrayDestroy(types);
    if (!alloc_method)
    {
        FR_ERROR("GetMethod(\"Alloc\") (0x%X)", hr);
        return 0;
    }

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(str);

    VARIANT v2;
    VariantInit(&v2);
    V_VT(&v2) = VT_INT;
    V_INT(&v2) = GCHandleType_Pinned;

    SAFEARRAY* params = get_params(&v, &v2);
    hr = alloc_method->Invoke_3(v, params, &v2);
    SysFreeString(V_BSTR(&v));
    SafeArrayDestroy(params);
    if (FAILED(hr))
    {
        FR_ERROR("Invoke(\"Alloc\") (0x%X)", hr);
        return 0;
    }

    VARIANT *gc_handle_v = (VARIANT *)malloc(sizeof(VARIANT));
    memcpy(gc_handle_v, &v2, sizeof(VARIANT));
    *gc_handle = (intptr_t)gc_handle_v;

    _MethodInfoPtr addr_of_pinned_object_method = 0;
    BSTR addrofpinned_b = SysAllocString(L"AddrOfPinnedObject");
    hr = gchandle_type->GetMethod_6(addrofpinned_b, &addr_of_pinned_object_method);
    SysFreeString(addrofpinned_b);
    if (!addr_of_pinned_object_method)
    {
        FR_ERROR("GetMethod(\"AddrOfPinnedObject\") (0x%X)", hr);
        return 0;
    }

    SAFEARRAY* get_func_args = SafeArrayCreateVector(VT_EMPTY, 0, 0);
    hr = addr_of_pinned_object_method->Invoke_3(v2, get_func_args, &v);
    SafeArrayDestroy(get_func_args);
    if (FAILED(hr))
    {
        FR_ERROR("Invoke(\"AddrOfPinnedObject\") (0x%X)", hr);
        return 0;
    }

    return V_INT(&v) - 0x8;
}

static inline ICorRuntimeHost* init_clr_runtime_host(LPCWSTR sz_runtimeVersion) {
    ICLRRuntimeInfo* pRuntimeInfo = NULL;
    ICorRuntimeHost* pRuntimeHost = NULL;
    ICLRMetaHost* pMetaHost = NULL;
    BOOL bLoadable;
    HMODULE hMscoree = GetModuleHandleA("mscoree.dll");
    if (hMscoree == 0)
        return NULL;
    CLRCreateInstanceFnPtr CLRCreateInstance = (CLRCreateInstanceFnPtr)GetProcAddress(hMscoree, "CLRCreateInstance");
    if (!CLRCreateInstance)
        return NULL;
    if (FAILED(CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (VOID**)&pMetaHost)))
    {
        FR_ERROR("CLRCreateInstance");
        return NULL;
    }
    if (FAILED(pMetaHost->GetRuntime(sz_runtimeVersion, IID_ICLRRuntimeInfo, (VOID**)&pRuntimeInfo))) {
        FR_ERROR("GetRuntime failed: %S", sz_runtimeVersion);
        return NULL;
    }
    if (FAILED(pRuntimeInfo->IsLoadable(&bLoadable)) || !bLoadable) {
        FR_ERROR("IsLoadable");
        return NULL;
    }
    if (FAILED(pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (VOID**)&pRuntimeHost))) {
        FR_ERROR("GetInterface");
        return NULL;
    }
    if (FAILED(pRuntimeHost->Start())) {
        FR_ERROR("Start");
        return NULL;
    }
    return pRuntimeHost;
}

static inline _AppDomainPtr get_default_domain(ICorRuntimeHost* pRuntimeHost) {
    IUnknownPtr pAppDomainThunk = NULL;
    if (FAILED(pRuntimeHost->GetDefaultDomain(&pAppDomainThunk))) {
        FR_ERROR("GetDefaultDomain");
        return NULL;
    }
    _AppDomainPtr pDefaultAppDomain = NULL;
    if (FAILED(pAppDomainThunk->QueryInterface(__uuidof(_AppDomain), (LPVOID*)&pDefaultAppDomain))) {
        FR_ERROR("QueryInterface");
        return NULL;
    }
    return pDefaultAppDomain;
}

static inline _AssemblyPtr get_assembly(_AppDomainPtr pDefaultAppDomain, const wchar_t *assembly)
{
    BSTR assembly_b = SysAllocString(assembly);
    _AssemblyPtr pAssembly = 0;
    HRESULT hr = pDefaultAppDomain->Load_2(assembly_b, &pAssembly);
    SysFreeString(assembly_b);
    if (!pAssembly)
    {
        FR_ERROR("Load (0x%X)", hr);
        return NULL;
    }
    return pAssembly;
}

bool init_clrhost()
{
    ICorRuntimeHost* pRuntimeHost = init_clr_runtime_host(L"v4.0.30319");
    if (!pRuntimeHost)
        return false;
    _MethodInfoPtr pMethodInfo = NULL;
    if (auto pDefaultAppDomain = get_default_domain(pRuntimeHost))
    {
        mscorlib_assembly = get_assembly(pDefaultAppDomain, L"mscorlib.dll");
        if (!mscorlib_assembly)
            FR_ERROR("Get mscorlib.dll Assembly Failed");
        osu_assembly = get_assembly(pDefaultAppDomain, L"osu!");
        if (!osu_assembly)
            FR_ERROR("Get osu! Assembly Failed");
        return false;
    }
    FR_ERROR("C# Get Default Domain Failed");
    return false;
}
