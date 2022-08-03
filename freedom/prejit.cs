using System;
using System.Reflection;

namespace Freedom
{
    public struct ClassMethod
    {
        public ClassMethod(String c, String m)
        {
            class_ = c;
            method = m;
        }

        public String class_ { get; set; }
        public String method { get; set; }
    }

    public class PreJit
    {
        public static int main(String pwzArgument)
        {
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            ClassMethod[] classmethods = new ClassMethod[]{
                new ClassMethod {class_ = "#=zZ86rRc_XTEYCVjLiIpwW9hgO85GX", method = "#=zaGN2R64="},
                new ClassMethod {class_ = "#=zZ86rRc_XTEYCVjLiIpwW9hgO85GX", method = "#=z28e6_TM="},
            };
            foreach (ClassMethod cm in classmethods)
            {
                foreach (Type class_ in classes)
                {
                    if (class_.Name == cm.class_)
                    {
                        MethodInfo[] methods = class_.GetMethods(
                                BindingFlags.DeclaredOnly |
                                BindingFlags.NonPublic |
                                BindingFlags.Public |
                                BindingFlags.Instance |
                                BindingFlags.Static);
                        foreach (MethodInfo method in methods)
                        {
                            if (method.Name == cm.method)
                            {
                                System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(method.MethodHandle);
                                Console.WriteLine(String.Format("prejit {0}::{1}", cm.class_, cm.method));
                            }
                        }
                    }
                }
            }
            return 1;
        }
    }
}
