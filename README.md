# rtgrader
A small autograder that compares user executable output to the reference compiler's output.
Output is generated using the "-r" flag for both the user and reference compiler.

## Known Bugs
hw7 and hw9 contain unrecoverable errors in the run-time compiler and are not excluded from the run-all methods.

# Setup
Download this folder and copy it into your compilers/template/ folder.
Run 'make' from the rtgrader directory to compile the autograder and cacher.

## File Structure
When installed correctly, the following folders should be present:
    template/rtgrader
    template/rt
    template/grader

The user executable file must be in the template folder, e.g. */template/jplc
The run-time executable file must be in the rt folder, e.g. */template/rt/jplc-linux

### Expected
In the rtgrader directory there is an **expected/** folder, which is necessary to run the autograder.
The expected outputs from the run-time compiler are contained in sub-folders of the **expected/** folder corresponding to each homework assignment.
If any folder or file is missing, the autograder will fail when attempting to check that output, in which case it needs to be re-cached.
In the event that a grader or runtime update is provided, the expected folder might need to be re-cached.
Currently, only hw's 2, 3, 4, 5, 6, 8, 10, and 11 are included.

# Makefile
All operations can be run using the provided makefile in the rtgrader directory.

## Commands
    make
        Compiles the **rtgrader** and **cacher** executables if their source files have been updated.

    make run [EXE] [TEST]
        *make run EXE=jplc TEST=hw2*

        Runs the **rtgrader** executable, testing the user's compiler output vs the expected run-time output for all programs in a given assignment.
        EXE: The user executable file-name in the **template/** folder. Defaults to **jplc**.
        TEST: The assignment folder to be tested. Defaults to **hw2**. Must correspond to a homework assignment (e.g. 'hw8'); the grader is not     compatible with sub-folders.

    make run-all [EXE]
        *make run-all EXE=jplc*

        Runs the **rtgrader** executable against every homework folder (excluding faulty folders).
        EXE: The user executable file-name in the **template/** folder. Defaults to **jplc**.

    make cache [RT] [TEST]
        *make cache RT=jplc-linux [TEST]=hw2*

        Runs the **cacher** executable, caching the expected output generated by the run-time compiler for all jpl programs in a given assignment.
            Files are kept in the **rtgrader/expected/** directory.
        RT: The run-time executable file-name in the **rt/** folder. Defaults to **jplc-linux**.
        TEST: The assignment folder to be tested. Defaults to **hw2**. Must correspond to a homework assignment (e.g. 'hw8'); the grader is not     compatible with sub-folders.

    make cache-all [RT]
        *make cache-all RT=jplc-linux*

        Runs the **cacher** executable for all homework folders (excluding faulty folders).
        RT: The run-time executable file-name in the **rt/** folder. Defaults to **jplc-linux**.

    make clean
        *make clean*

        Cleans up the **rtgrader/** directory. Leaves the **expected/** folder and its contents intact.
            Deletes the **cacher** and **rtgrader** executables.

    make clean-expected
        *make clean-expected*

        Deletes the **expected/** folder and all of its contents.
            Due to the slow speed of the run-time compiler, regenerating this folder is not recommended if avoidable.
