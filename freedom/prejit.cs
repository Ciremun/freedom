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
        CheckTime = 5
    }

    struct ClassMethod
    {
        public int c { get; set; }
        public int m { get; set; }
        public ClassMethodType t { get; set; }
    }

    public struct ClassMethodF
    {
        public String c { get; set; }
        public String m { get; set; }
        public ClassMethodType t { get; set; }
    }

    public class PreJit
    {
        public static int prejit_all_f(String s)
        {
            ClassMethodF[] classmethods = new ClassMethodF[]{
                new ClassMethodF {c = "#=zOTWUr4vq60U15SRmD_JItyatbhdR", m = "#=zQ0BzTMmHMslK", t = ClassMethodType.Load},
                new ClassMethodF {c = "#=zImjlgOWc5_OOhYavAOGdufVmsem_hUPRKq75Spg=", m = "#=z3EFQLcObB3UFr7eHXw==", t = ClassMethodType.Replay},
                new ClassMethodF {c = "#=zH0M3f8p46pS8pXcQsRD98exEqdyNHOTUSsRVX0w=", m = "#=zmSXoRp0Eyp4Q", t = ClassMethodType.Score},
                new ClassMethodF {c = "#=zOTWUr4vq60U15SRmD_JItyatbhdR", m = "#=z9D9oHDkdx1V_oPJ5DVS3F_Y=", t = ClassMethodType.CheckFlashlight},
                new ClassMethodF {c = "#=zmkr0gBO9O_1YsQB4j4wz5GMMCTsyQoSERtD4iXqQ4yQUe4gn8Q==", m = "#=zEfzFcsIXc9HpZcy7rQ==", t = ClassMethodType.UpdateFlashlight},
                new ClassMethodF {c = "#=zOTWUr4vq60U15SRmD_JItyatbhdR", m = "#=z_S0hF4Y=", t = ClassMethodType.CheckTime}
            };
            int ret = 1;
            var assembly = Assembly.GetEntryAssembly();
            foreach (ClassMethodF cm in classmethods)
            {
                try
                {
                    var c = assembly.GetType(cm.c);
                    MethodInfo m = null;
                    if (cm.t == ClassMethodType.Score) {
                        foreach (MethodInfo me in c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static)) {
                            if (me.Name == cm.m) {
                                foreach (ParameterInfo p in me.GetParameters()) {
                                    if (p.Name == "#=znuwh7sAl6Yx2") {
                                        m = me;
                                    }
                                }
                            }
                        }
                    }
                    if (m == null)
                        m = c.GetMethod(cm.m, BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
                    System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(m.MethodHandle);
                } catch (Exception) { ret = 0; }
            }
            return ret;
        }
        public static int prejit_all(String s)
        {
            ClassMethod[] classmethods = new ClassMethod[]{
                new ClassMethod {c = 31, m = 15, t = ClassMethodType.Load},
                new ClassMethod {c = 43, m = 23, t = ClassMethodType.Replay},
                new ClassMethod {c = 43, m = 15, t = ClassMethodType.Score},
                new ClassMethod {c = 31, m = 27, t = ClassMethodType.CheckFlashlight},
                new ClassMethod {c = 55, m = 23, t = ClassMethodType.UpdateFlashlight},
                new ClassMethod {c = 31, m = 11, t = ClassMethodType.CheckTime},
            };
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            foreach (Type c in classes)
            {
                MethodInfo[] methods = c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
                foreach (MethodInfo m in methods)
                {
                    foreach (ClassMethod cm in classmethods)
                    {
                        if (c.Name.Length == cm.c && m.Name.Length == cm.m)
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
