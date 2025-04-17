ground object System {
    open group out {
        open void native printnl(const char* s) {
            System.read.native("/workspaces/Covione/CoviSys/NativeLib/libCovio.so")
            libCovio.printnl(s)
        }
    }
}
// use covicc and compile this file to Covio.covil