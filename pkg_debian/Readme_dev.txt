This is documentation on Building the Deb pkg

**
    if u have a new sw_exe , ser_exe , cli_exe or ClientUI
    Run rebuild.sh script under helper scripts
**

the opt/gobackn/lib folder conatains the .so files of qmake on which my gui exe ClientUI depends
U can find the dependcies of any exe through ldd fileName command

**
    Every exe have dependcies(.so)
    -> standarad system .so like libc or libm [Mention them under Depends section in control file under DEBIAN/control]
    -> 3rd party like Qt which I am bundling inside lib folder and setting LD_LIBRARY_PATH through a wrapper script of  gui exe ie instead of directly running the gui exe one has to run this srcipt
**

**
    building the pkg (inside pkg_debian folder):-
        dpkg-deb --build udp-ft-suite
**

**
    installing the pkg (inside pkg_debian folder):-
        sudo apt install ./udp-ft-suite.deb
**

**
    Removing the pkg (from anywhere):-
        sudo apt purge udp-ft-suite
**

--> Dont forget to do this before rebuilding the deb package
prtihijit@hp-250-g6:~/Desktop/go-back-n/pkg_debian/udp-ft-suite$
chmod 777 opt/gobackn/*
chmod +x DEBIAN/postrm
chmod +x DEBIAN/postinst

### How to identify which shared lib objects are mandatory for GUI to function
and which to place them under the lib folder.

do ->  ldd gui_exe   [copy all the Qt .so files ie Not present in /lib or /lib64]
do ->   QT_DEBUG_PLUGINS=1 gui_exe [check for unloaded libaries when u close gui]

Maintain strict folder structure for plugins

Certain plugins also have dependcies keep them under lib

plugins .so will not appear through ldd dump

----------------------------------- For Older Distros  ----------------------
--> Depends section of control file Does the following :-
Depends: libfoo (>= 1.2)
...and the system:
    Has no libfoo installed

    or has libfoo < 1.2

    but libfoo >= 1.2 exists in the repositories... [upadte through sudo apt upadte]

✅ apt will automatically install or upgrade libfoo to meet the requirement.


/*  I am even packing my own linker and all the .so files in lib So that on
older versions of distros No [Undefined Symbol error or Package Not found error] ocuurs
As Older linker Cant work with New so files */

[To pack dependcies use copy_lib.sh under helper scripts]

--> Building on older versions that exe can run of New versions of Ubuntu

// Dont upgrade glibc [system core package] may break ur OS
