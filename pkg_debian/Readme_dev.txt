This is a documentation on Building the Deb pkg

**
    if u have a new sw_exe , ser_exe , cli_exe or ClientUI
    1. ldd *_exe > ldd_output_file
    2. helper_scripts/copy_lib.sh ldd_output_file
    3. helper_scripts/rebuild.sh
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

____________________________________________________________________
--> Dont forget to do this before 1st time building the deb package
prtihijit@hp-250-g6:~/Desktop/go-back-n/pkg_debian/udp-ft-suite$
chmod 777 opt/gobackn/*
chmod +x DEBIAN/postrm
chmod +x DEBIAN/postinst

dpkg detects these perm and keep it
*****************************************************************************
Story behind :-
the opt/gobackn/lib folder conatains the .so files & linker used by my exe


--> Issue:- On building & packing exe on Ubuntu 24 , the exe dont run in Ubuntu 22 due to version mismatch
of glic.so (core system package) version not found error ie on Ubuntu 22 user trying to install .deb pkg
can't use as myexe demands lets say version6 but available5. Cant even upgrade
the version [Note difference between update and upgrade] as version6 is for Ubuntu24
so to upgrade from 5 to 6 u will need new distro version.

--> Soln Pack ALL the dependcies (.so) and even my linker(ld-linux.so) along with
executable, And provide a script which sets this environment of linker and linker_library_path
for our exe to run ==> Whenever user wants to run my exe, he has to run my script.[Command line args]
passed to scripts are given to exe . ie for user he only see that exe name changed to script name.

--> Now why even pack my linker? linux-linker is tightly coupled with glibc. So using
Host linker(old) with my new (.so) files will lead to symbol not found error.

---> Note dependcies management through depends of control file is limited to the version
of Ubuntu where pkg & exe is build. ie as I am on Ubuntu24 if I Mention my dependcies in contol file
and allow dpkg to take care of it then my deb package will work on Ubuntu version >=24

---> Ie One Soln includes Building pkg on oldest possible distro --> prevents the hassle
of manually packing.

###### Now with my deb setup my pkg has become Ubuntu version independent

************************************************************************************

----------------------------------GUI dependcies detect---------------------------------------------

### How to identify which shared lib objects are mandatory for myGUI to function
and where to place them under the lib folder##

do ->  ldd gui_exe   [copy all the Qt .so files ie Not present in /lib or /lib64]
do ->   QT_DEBUG_PLUGINS=1 gui_exe [check for unloaded libaries when u close gui]

Maintain strict folder structure for plugins

Certain plugins also have dependcies keep them under lib

plugins .so will not appear through ldd dump [as plugins .so are runtime loadable]
No info of them in elf exe that ldd can read

----------------------------------- Function of Depends section of control file  ----------------------

--> Depends section of control file Does the following :-
Depends: libfoo (>= 1.2)
...and the system:
    Has no libfoo installed

    or has libfoo < 1.2

    but libfoo >= 1.2 exists in the repositories... [upadte through sudo apt upadte]

✅ apt will automatically install or upgrade libfoo to meet the requirement.
But apt will upgrade libfoo to only the version supported by the version of host Ubuntu
