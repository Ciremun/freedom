// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Freedom
{
    public struct ClassMethod
    {
        public ClassMethod(String c, String m, String n)
        {
            class_ = c;
            method = m;
            name = n;
        }

        public String class_ { get; set; }
        public String method { get; set; }
        public String name { get; set; }
    }

    public class SetPresence
    {
        public static int GetSetPresencePtr(String pwzArgument)
        {
            return (int)Assembly.GetEntryAssembly().GetType("DiscordRPC.DiscordRpcClient").GetMethod("SetPresence").MethodHandle.GetFunctionPointer();
        }
        public static int GetCSharpStringPtr(String pwzArgument)
        {
            GCHandle handle = GCHandle.Alloc(pwzArgument, GCHandleType.Pinned);
            return (int)(handle.AddrOfPinnedObject() - 0x8);
        }
    }

    public class PreJit
    {
        public static ClassMethod[] classmethods = new ClassMethod[]{
            new ClassMethod {class_ = "#=z_JoBdjCdwD3UqdPyMDTdmtx7HG8J", method = "#=zpbRFhHIGLFX4", name="beatmap_onload"},
            new ClassMethod {class_ = "#=zuzcYy$AnALKJhx0RlLp1l4ahmCVSgkWbMNkerfg=", method = "#=z$hHktWjcmnjerZy8LA==", name="selected_replay"},
            new ClassMethod {class_ = "#=zA68w2LnfHk3bAvNoTjj7pqCRs0P7Q2WkMrK0LXo=", method = "#=z_gY4$2rMOiN4", name="score_multiplier"},
        };
        public static int prejit_all(String pwzArgument)
        {
            Console.WriteLine("start prejit all");
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            foreach (Type class_ in classes)
            {
                MethodInfo[] methods = class_.GetMethods(
                        BindingFlags.DeclaredOnly |
                        BindingFlags.NonPublic |
                        BindingFlags.Public |
                        BindingFlags.Instance |
                        BindingFlags.Static);
                foreach (MethodInfo method in methods)
                {
                    foreach (ClassMethod cm in classmethods)
                    {
                        if (class_.Name.Length == cm.class_.Length && method.Name.Length == cm.method.Length)
                        {
                            try
                            {
                                System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(method.MethodHandle);
                            } catch (Exception) {}
                        }
                    }
                }
            }
            Console.WriteLine("done prejit all");
            return 1;
        }
    }
}
