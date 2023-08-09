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
            new ClassMethod {c = "#=zOTWUr4vq60U15SRmD_JItyatbhdR", m = "#=zQ0BzTMmHMslK", t = ClassMethodType.Load},
            new ClassMethod {c = "#=zImjlgOWc5_OOhYavAOGdufVmsem_hUPRKq75Spg=", m = "#=z3EFQLcObB3UFr7eHXw==", t = ClassMethodType.Replay},
            new ClassMethod {c = "#=zH0M3f8p46pS8pXcQsRD98exEqdyNHOTUSsRVX0w=", m = "#=zmSXoRp0Eyp4Q", t = ClassMethodType.Score},
            new ClassMethod {c = "#=zOTWUr4vq60U15SRmD_JItyatbhdR", m = "#=z9D9oHDkdx1V_oPJ5DVS3F_Y=", t = ClassMethodType.CheckFlashlight},
            new ClassMethod {c = "#=zmkr0gBO9O_1YsQB4j4wz5GMMCTsyQoSERtD4iXqQ4yQUe4gn8Q==", m = "#=zEfzFcsIXc9HpZcy7rQ==", t = ClassMethodType.UpdateFlashlight},
            new ClassMethod {c = "#=zOTWUr4vq60U15SRmD_JItyatbhdR", m = "#=z_S0hF4Y=", t = ClassMethodType.CheckTime},
            new ClassMethod {c = "#=z$pwtjhUA_dQrRuSobUzK5ZhNV8JB", m = "    ​   ​  ​  ​ ", t = ClassMethodType.UpdateVariables},
        };

        static MethodInfo find_score_method(Type c, String cm_m)
        {
            foreach (MethodInfo me in c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static)) {
                if (me.Name == cm_m) {
                    foreach (ParameterInfo p in me.GetParameters()) {
                        if (p.Name == "#=znuwh7sAl6Yx2") {
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
                        if (c.Name.Length == cm.c.Length && m.Name.Length == cm.m.Length)
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
            return 1;
        }
    }
}
