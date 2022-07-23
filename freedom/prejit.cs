using System;
using System.Reflection;

namespace Freedom
{
    public class PreJit
    {
        public static int main(String pwzArgument)
        {
            var assembly = Assembly.GetEntryAssembly();
            Type[] classes = assembly.GetTypes();
            foreach (Type class_ in classes)
            {
                if (class_.Name == "#=zZ86rRc_XTEYCVjLiIpwW9hgO85GX")
                {
                    MethodInfo[] methods = class_.GetMethods(
                            BindingFlags.DeclaredOnly |
                            BindingFlags.NonPublic |
                            BindingFlags.Public |
                            BindingFlags.Instance |
                            BindingFlags.Static);
                    foreach (MethodInfo method in methods)
                    {
                        if (method.Name == "#=zaGN2R64=")
                        {
                            System.Runtime.CompilerServices.RuntimeHelpers.PrepareMethod(method.MethodHandle);
                            Console.WriteLine("prejit beatmap_onload");
                        }
                    }
                }
            }
            return 1;
        }
    }
}
