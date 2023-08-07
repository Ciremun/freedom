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

    enum ClassMethodType : int
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
        ClassMethod(int c_, int m_, int n_, ClassMethodType t_)
        {
            c = c_;
            m = m_;
            n = n_;
            t = t_;
        }

        public int c { get; set; }
        public int m { get; set; }
        public int n { get; set; }
        public ClassMethodType t { get; set; }
    }

    public class PreJit
    {
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
                MethodInfo[] methods = c.GetMethods(
                        BindingFlags.DeclaredOnly |
                        BindingFlags.NonPublic |
                        BindingFlags.Public |
                        BindingFlags.Instance |
                        BindingFlags.Static);
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
