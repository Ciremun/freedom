// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Freedom
{
    public class SetPresence
    {
        public static int GetSetPresencePtr(String s)
        {
            return (int)Assembly.GetEntryAssembly().GetType("DiscordRPC.DiscordRpcClient").GetMethod("SetPresence").MethodHandle.GetFunctionPointer();
        }
        public static int GetCSharpStringPtr(String s)
        {
            GCHandle handle = GCHandle.Alloc(s, GCHandleType.Pinned);
            return (int)(handle.AddrOfPinnedObject() - 0x8);
        }
    }

    public enum ClassMethodType : int
    {
        Load = 0,
        Replay = 1,
        Score = 2,
        CheckFlashlight = 3,
        UpdateFlashlight = 4,
        CheckTime = 5,
        UpdateVariables = 6
    }

    public struct ClassMethod
    {
        public String c { get; set; }
        public String m { get; set; }
        public ClassMethodType t { get; set; }
    }

    public class PreJit
    {
        static ClassMethod[] classmethods = new ClassMethod[]{
            new ClassMethod {c = "#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q", m = "#=zanE7Qv3XmjW5", t = ClassMethodType.Load},
            new ClassMethod {c = "#=zdKJOjAyV$ER12ZW5$R$q$GYWyVHooHal7eJTMqI=", m = "#=zz6ecxM6_4e4c6Im$hA==", t = ClassMethodType.Replay},
            new ClassMethod {c = "#=zZxV46FKXz8HxVteOcXctYSVtWa6VKIH_YNsHAm8=", m = "#=zNFUNmEy9GMdn", t = ClassMethodType.Score},
            new ClassMethod {c = "#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q", m = "#=z8sU_nCrclRWZEfIsFhlq2S4=", t = ClassMethodType.CheckFlashlight},
            new ClassMethod {c = "#=z3onHVu8ArxqQRwlNg7LafPTccV82AY6DfUBBuhGZ$HyZ0LAnfQ==", m = "#=zn$KM8OPB3VD3EEQhXA==", t = ClassMethodType.UpdateFlashlight},
            new ClassMethod {c = "#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q", m = "#=zr__5T0o=", t = ClassMethodType.CheckTime},
            new ClassMethod {c = "#=zE0VDZfwJEH3z6D3XGqmTkDRFkwfG", m = "", t = ClassMethodType.UpdateVariables},
        };

        unsafe delegate void ClassMethodsFromAddrsDelegate(Int32 *cms, Int32 size);
        static GCHandle delegate_handle;

        public static int SetClassMethod(String classmethod)
        {
            String[] class_method_type = classmethod.Split(new string[] {"::"}, StringSplitOptions.None);
            if (class_method_type.Length < 3)
                return 0;
            String class_ = class_method_type[0];
            String method = class_method_type[1];
            String type_ = class_method_type[2];
            for (Int32 i = 0; i < classmethods.Length; i++)
            {
                if (classmethods[i].t == (ClassMethodType)Enum.Parse(typeof(ClassMethodType), type_))
                {
                    classmethods[i].c = class_;
                    classmethods[i].m = method;
                    break;
                }
            }
            return 1;
        }

        unsafe public static int GetClassMethodsFromAddrsPtr(string s)
        {
            Delegate d = new ClassMethodsFromAddrsDelegate(ClassMethodsFromAddrs);
            delegate_handle = GCHandle.Alloc(d);
            return Marshal.GetFunctionPointerForDelegate(d).ToInt32();
        }

        unsafe public static void ClassMethodsFromAddrs(Int32 *cms, Int32 size)
        {
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            foreach (Type c in classes)
            {
                MethodInfo[] methods = c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
                foreach (MethodInfo m in methods)
                {
                    for (Int32 i = 0, j = 0; i < size; i += 1, j += 5)
                    {
                        // NOTE(Ciremun): method was already found
                        if (*(IntPtr *)(cms + j + 4) == (IntPtr)1)
                            continue;
                        if (cms[j + 2] == (Int32)ClassMethodType.UpdateVariables)
                        {
                            try
                            {
                                if (c.Name.Length == "#=zlCdNwFnwVPuuVVN9zWDRLlYYPsa5".Length)
                                {
                                    if (m.ReturnType == typeof(void))
                                    {
                                        ParameterInfo[] parameters = m.GetParameters();
                                        if (parameters.Length == 3)
                                        {
                                            if (parameters[0].ParameterType == typeof(System.Boolean) &&
                                                parameters[1].ParameterType == typeof(System.Boolean) &&
                                                parameters[2].ParameterType == typeof(System.Boolean))
                                            {
                                                cms[j] = SetPresence.GetCSharpStringPtr(c.Name) + 0x8;
                                                cms[j + 1] = SetPresence.GetCSharpStringPtr(m.Name) + 0x8;
                                                *(IntPtr *)(cms + j + 4) = (IntPtr)1;
                                            }
                                        }
                                    }
                                }
                            }
                            catch (Exception) { }
                            continue;
                        }
                        IntPtr method_addr = *(IntPtr *)(cms + j + 3);
                        IntPtr iter_method_addr = IntPtr.Zero;
                        try { iter_method_addr = m.MethodHandle.GetFunctionPointer(); }
                        catch (Exception) { break; }
                        if (method_addr == iter_method_addr)
                        {
                            cms[j] = SetPresence.GetCSharpStringPtr(c.Name) + 0x8;
                            cms[j + 1] = SetPresence.GetCSharpStringPtr(m.Name) + 0x8;
                            *(IntPtr *)(cms + j + 4) = (IntPtr)1;
                        }
                    }
                }
            }
            delegate_handle.Free();
        }

        static MethodInfo find_score_method(Type c, String cm_m)
        {
            foreach (MethodInfo me in c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static)) {
                if (me.Name == cm_m) {
                    foreach (ParameterInfo p in me.GetParameters()) {
                        if (p.Name.Length == "#=znuwh7sAl6Yx2".Length) {
                            return me;
                        }
                    }
                }
            }
            return null;
        }

        public static int prejit_all_f(String s)
        {
            int ret = 1;
            var assembly = Assembly.GetEntryAssembly();
            foreach (ClassMethod cm in classmethods)
            {
                try
                {
                    var c = assembly.GetType(cm.c);
                    MethodInfo m = null;
                    if (cm.t == ClassMethodType.Score)
                        m = find_score_method(c, cm.m);
                    if (m == null)
                        m = c.GetMethod(cm.m, BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
                    System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(m.MethodHandle);
                } catch (Exception) { ret = 0; }
            }
            return ret;
        }

        public static int prejit_all(String s)
        {
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            foreach (Type c in classes)
            {
                MethodInfo[] methods = c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
                foreach (MethodInfo m in methods)
                {
                    foreach (ClassMethod cm in classmethods)
                    {
                        if (c.Name.Length == cm.c.Length)
                        {
                            if (m.Name.Length == cm.m.Length || cm.t == ClassMethodType.UpdateVariables)
                            {
                                if (cm.t == ClassMethodType.Load && c.IsSealed)
                                    continue;
                                try
                                {
                                    System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(m.MethodHandle);
                                } catch (Exception) {}
                            }
                        }
                    }
                }
            }
            return 1;
        }
    }
}
