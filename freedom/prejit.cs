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
            new ClassMethod {c = "#=z37QZypQr4nWO6NMndLUZbwX2sS9c", m = "#=zb1jGoyFFrOX0", t = ClassMethodType.Load},
            new ClassMethod {c = "#=zxAdngeK4jmaF$2DYJq1i9CETT$MQwhE5MSaDWyQ=", m = "#=zZ7ig4tKjfzs99vmAXw==", t = ClassMethodType.Replay},
            new ClassMethod {c = "#=zpVkvZItSd0$JNJ805MQDn_GSppZbxgi7ceg$6kk=", m = "#=z5ToczxPG8UN9", t = ClassMethodType.Score},
            new ClassMethod {c = "#=z37QZypQr4nWO6NMndLUZbwX2sS9c", m = "#=zhK9AFFYGeb1Si9rTLeDsIBg=", t = ClassMethodType.CheckFlashlight},
            new ClassMethod {c = "#=zGGmgZsNHh$I6KZ5FJe1B9YWAbsD1K2Uy0elN3uao0H8iu8aAZQ==", m = "#=z8xtAaXlWH3V6LMsElg==", t = ClassMethodType.UpdateFlashlight},
            new ClassMethod {c = "#=z37QZypQr4nWO6NMndLUZbwX2sS9c", m = "#=zdqGVlv4=", t = ClassMethodType.CheckTime},
            new ClassMethod {c = "#=zhwn9CnvJGl36$lSN1VJUr9grE30G", m = "              ", t = ClassMethodType.UpdateVariables},
        };

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
