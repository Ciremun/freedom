#nullable enable

using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace Freedom
{
    public class Utils
    {
        enum ClassMethodType : int
        {
            Load = 0,
            Replay = 1,
            Score = 2,
            CheckFlashlight = 3,
            UpdateFlashlight = 4,
            CheckTime = 5,
            UpdateVariables = 6
        }

        struct ClassMethod
        {
            public String c;
            public TypeAttributes ca;
            public String m;
            public MethodAttributes ma;
            public int mpc;
            public int[]? mpnl;
            public int[]? mptl;
            public Type mrt;
            public ClassMethodType t;
        }

        static ClassMethod[] classmethods = new ClassMethod[]{
            new ClassMethod {
                c = "#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class,
                m = "#=zanE7Qv3XmjW5",
                ma = MethodAttributes.PrivateScope | MethodAttributes.Private | MethodAttributes.HideBySig | MethodAttributes.ReuseSlot,
                mpc = 0,
                mpnl = null,
                mptl = null,
                mrt = typeof(void),
                t = ClassMethodType.Load,
            },
            new ClassMethod {
                c = "#=zdKJOjAyV$ER12ZW5$R$q$GYWyVHooHal7eJTMqI=",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class,
                m = "#=zz6ecxM6_4e4c6Im$hA==",
                ma = MethodAttributes.PrivateScope | MethodAttributes.Family | MethodAttributes.Virtual | MethodAttributes.HideBySig |
                     MethodAttributes.VtableLayoutMask | MethodAttributes.ReuseSlot | MethodAttributes.NewSlot,
                mpc = 0,
                mpnl = null,
                mptl = null,
                mrt = typeof(void),
                t = ClassMethodType.Replay,
            },
            new ClassMethod {
                c = "#=zZxV46FKXz8HxVteOcXctYSVtWa6VKIH_YNsHAm8=",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class | TypeAttributes.Abstract | TypeAttributes.Sealed,
                m = "#=zNFUNmEy9GMdn",
                ma = MethodAttributes.PrivateScope | MethodAttributes.Private | MethodAttributes.FamANDAssem | MethodAttributes.Assembly |
                     MethodAttributes.Static | MethodAttributes.HideBySig | MethodAttributes.ReuseSlot,
                mpc = 3,
                mpnl = new int[] { 11, 11, 15 },
                mptl = new int[] { 15, 20, 47 },
                mrt = typeof(System.Double),
                t = ClassMethodType.Score,
            },
            new ClassMethod {
                c = "#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class,
                m = "#=z8sU_nCrclRWZEfIsFhlq2S4=",
                ma = MethodAttributes.PrivateScope | MethodAttributes.Private | MethodAttributes.HideBySig | MethodAttributes.ReuseSlot,
                mpc = 0,
                mpnl = null,
                mptl = null,
                mrt = typeof(void),
                t = ClassMethodType.CheckFlashlight,
            },
            new ClassMethod {
                c = "#=z3onHVu8ArxqQRwlNg7LafPTccV82AY6DfUBBuhGZ$HyZ0LAnfQ==",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class,
                m = "#=zn$KM8OPB3VD3EEQhXA==",
                ma = MethodAttributes.PrivateScope | MethodAttributes.Private | MethodAttributes.FamANDAssem | MethodAttributes.Assembly |
                     MethodAttributes.Virtual | MethodAttributes.HideBySig | MethodAttributes.CheckAccessOnOverride | MethodAttributes.ReuseSlot,
                mpc = 0,
                mpnl = null,
                mptl = null,
                mrt = typeof(void),
                t = ClassMethodType.UpdateFlashlight,
            },
            new ClassMethod {
                c = "#=zI8KegJG$4iW$48IFM8jc7BaeZl5Q",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class,
                m = "#=zr__5T0o=",
                ma = MethodAttributes.PrivateScope | MethodAttributes.FamANDAssem | MethodAttributes.Family | MethodAttributes.Public |
                     MethodAttributes.Virtual | MethodAttributes.HideBySig | MethodAttributes.ReuseSlot,
                mpc = 0,
                mpnl = null,
                mptl = null,
                mrt = typeof(void),
                t = ClassMethodType.CheckTime,
            },
            new ClassMethod {
                c = "#=zE0VDZfwJEH3z6D3XGqmTkDRFkwfG",
                ca = TypeAttributes.NotPublic | TypeAttributes.AutoLayout | TypeAttributes.Class | TypeAttributes.Abstract,
                m = "",
                ma = MethodAttributes.PrivateScope | MethodAttributes.Private | MethodAttributes.FamANDAssem | MethodAttributes.Assembly |
                     MethodAttributes.Virtual | MethodAttributes.HideBySig | MethodAttributes.CheckAccessOnOverride |
                     MethodAttributes.VtableLayoutMask | MethodAttributes.ReuseSlot | MethodAttributes.NewSlot,
                mpc = 3,
                mpnl = new int[] { 23, 23, 27 },
                mptl = new int[] { 14, 14, 14 },
                mrt = typeof(void),
                t = ClassMethodType.UpdateVariables,
            },
        };

        static Dictionary<String, GCHandle> CSharpStringHandles = new Dictionary<String, GCHandle>();

        public static int GetSetPresencePtr()
        {
            return (int)Assembly.GetEntryAssembly().GetType("DiscordRPC.DiscordRpcClient").GetMethod("SetPresence").MethodHandle.GetFunctionPointer();
        }

        public static int GetCSharpStringPtr(String s)
        {
            if (CSharpStringHandles.ContainsKey(s))
                return (int)(CSharpStringHandles[s].AddrOfPinnedObject() - 0x8);
            GCHandle handle = GCHandle.Alloc(s, GCHandleType.Pinned);
            CSharpStringHandles.Add(s, handle);
            return (int)(handle.AddrOfPinnedObject() - 0x8);
        }

        public static int FreeCSharpString(String s)
        {
            if (CSharpStringHandles.ContainsKey(s))
            {
                CSharpStringHandles[s].Free();
                CSharpStringHandles.Remove(s);
                return 1;
            }
            return 0;
        }

        static bool MatchClassNameLength(Type c)
        {
            foreach (ClassMethod cm in classmethods)
                if (c.Name.Length == cm.c.Length)
                    return true;
            return false;
        }

        // NOTE(Ciremun): Compare class and method attributes we are looping with our predefined classes and methods
        static bool VerifyClassMethod(Type c, MethodInfo m, ClassMethod cm)
        {
            TypeAttributes ca = c.Attributes;
            if ((ca & TypeAttributes.Abstract) != (cm.ca & TypeAttributes.Abstract)) return false;
            if ((ca & TypeAttributes.Sealed) != (cm.ca & TypeAttributes.Sealed))     return false;

            TypeAttributes v = ca & TypeAttributes.VisibilityMask;
            if ((v & TypeAttributes.NotPublic) != (cm.ca & TypeAttributes.NotPublic))                 return false;
            if ((v & TypeAttributes.Public) != (cm.ca & TypeAttributes.Public))                       return false;
            if ((v & TypeAttributes.NestedPublic) != (cm.ca & TypeAttributes.NestedPublic))           return false;
            if ((v & TypeAttributes.NestedPrivate) != (cm.ca & TypeAttributes.NestedPrivate))         return false;
            if ((v & TypeAttributes.NestedFamANDAssem) != (cm.ca & TypeAttributes.NestedFamANDAssem)) return false;
            if ((v & TypeAttributes.NestedAssembly) != (cm.ca & TypeAttributes.NestedAssembly))       return false;
            if ((v & TypeAttributes.NestedFamily) != (cm.ca & TypeAttributes.NestedFamily))           return false;
            if ((v & TypeAttributes.NestedFamORAssem) != (cm.ca & TypeAttributes.NestedFamORAssem))   return false;

            TypeAttributes l = ca & TypeAttributes.LayoutMask;
            if ((l & TypeAttributes.AutoLayout) != (cm.ca & TypeAttributes.AutoLayout))             return false;
            if ((l & TypeAttributes.SequentialLayout) != (cm.ca & TypeAttributes.SequentialLayout)) return false;
            if ((l & TypeAttributes.ExplicitLayout) != (cm.ca & TypeAttributes.ExplicitLayout))     return false;

            TypeAttributes cs = ca & TypeAttributes.ClassSemanticsMask;
            if ((cs & TypeAttributes.Class) != (cm.ca & TypeAttributes.Class))         return false;
            if ((cs & TypeAttributes.Interface) != (cm.ca & TypeAttributes.Interface)) return false;

            if (m.Attributes != cm.ma)  return false;
            if (m.ReturnType != cm.mrt) return false;

            ParameterInfo[] p = m.GetParameters();
            if (p.Length != cm.mpc) return false;

            for (int i = 0; i < cm.mpc; ++i)
                if (p[i].Name.Length != cm.mpnl![i] ||
                   (p[i].ParameterType.Name.Length != cm.mptl![i] && cm.t != ClassMethodType.UpdateVariables && cm.t != ClassMethodType.Score))
                    return false;

            return true;
        }

        public static int PrepareMethods()
        {
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            int i = 0;
            foreach (Type c in classes)
            {
                if (!MatchClassNameLength(c))
                    continue;
                MethodInfo[] methods = c.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
                foreach (MethodInfo m in methods)
                {
                    foreach (ClassMethod cm in classmethods)
                    {
                        if (m.Name.Length != cm.m.Length && cm.t != ClassMethodType.UpdateVariables)
                            continue;
                        if (!VerifyClassMethod(c, m, cm))
                            continue;
                        try
                        {
                            i += 1;
                            System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(m.MethodHandle);
                        } catch (Exception) {}
                    }
                }
            }
            return i;
        }
    }
}
