# MacoraOne

## Unified Command-Line System for the MacoraOne Ecosystem

MacoraOne is the command-line controller for the related Dominex Macedon development projects:

- `greenServeFE-build`
- `greenServeFE-`
- `CMLL-build`
- `CMLL`
- `NativeFlexi`
- `NativeFlexi-android`

The implementation is a native C program compiled to a Linux executable named `one`.

The controller deliberately uses the existing project repositories instead of replacing their build systems. The greenServeFE build repository keeps its C11/Makefile structure, CMLL keeps its C11 build structure, and NativeFlexi Android remains a conventional Java/Kotlin Android project with Gradle and GitHub Actions. The repository layouts inspected by MacoraOne are therefore the existing layouts, not a second custom project format.

## Build

Requirements:

- Linux
- GCC or another C11 compiler
- `make`
- `git`
- GitHub CLI (`gh`) for GitHub account, repository, push, and Actions operations
- A browser for GitHub authorization and artifact pages

Compile:

```text
make clean all
```

Test the executable:

```text
./one --version
./one --help
make test
```

Install the executable as the `one` command:

```text
sudo make install
```

Uninstall the executable:

```text
sudo make uninstall
```

## GitHub Authentication

`one create`, `one push`, and `one build` use the official GitHub CLI authentication state.

When no authenticated GitHub account is detected, MacoraOne asks:

```text
Do you have a GitHub account? [Y/n]:
```

If the answer is no, MacoraOne opens:

```text
https://github.com/signup
```

The developer must create the account first and then run the command again.

If the answer is yes, MacoraOne starts:

```text
gh auth login --hostname github.com --git-protocol https --web
```

This uses GitHub CLI's browser-based authorization flow rather than storing a GitHub password or token inside MacoraOne.

## `one install`

`one install` refreshes the local MacoraOne ecosystem installation.

The installation root is:

```text
~/.macoraone/
```

It contains:

```text
~/.macoraone/
├── components/
│   ├── greenServeFE-build/
│   ├── greenServeFE-/
│   ├── CMLL-build/
│   ├── CMLL/
│   ├── NativeFlexi/
│   └── NativeFlexi-android/
└── projects/
```

Existing copies are removed before the repositories are cloned again. This prevents stale or duplicated installed copies from being used.

After cloning, MacoraOne also attempts to build:

```text
greenServeFE-build -> greenServeFE
CMLL-build         -> cmll
```

## `one uninstall`

Remove the managed MacoraOne installation:

```text
one uninstall
```

The command removes `~/.macoraone/` and attempts to remove the currently executing `one` executable when it is able to resolve and delete it.

If `one` was installed into `/usr/local/bin`, use sufficient permissions when necessary.

## `one create AppName`

This is the main Android project creation workflow.

Example:

```text
one create WeatherApp
```

MacoraOne performs the following sequence:

```text
1. Check for GitHub CLI.
2. Check whether a GitHub account is already authenticated.
3. Ask whether the developer has a GitHub account if authentication is absent.
4. Open GitHub signup when the developer does not have an account.
5. Start GitHub browser authorization when an account exists.
6. Ask whether the Android project should use Kotlin or Java.
7. Refresh the NativeFlexi-android template from GitHub before every project creation.
8. Select the requested NativeFlexi template.
9. Copy only the selected Java or Kotlin project.
10. Copy only the matching GitHub Actions workflow.
11. Do not include the opposite Java/Kotlin project.
12. Keep normal Android files such as Gradle files, AndroidManifest.xml, Java/Kotlin source, resources, and workflow configuration.
13. Create the local AppName directory.
14. Initialize a Git repository.
15. Create a public GitHub repository named AppName.
16. Push the selected Android project to that repository.
```

The generated project is stored locally at:

```text
~/.macoraone/projects/AppName/
```

The generated project remains a normal Android project. MacoraOne does not introduce `.nfx` files, a custom UI language, a custom Android compiler, or a custom Android runtime.

## Java Selection

When Java is selected, MacoraOne uses:

```text
NativeFlexi-java/
.github/workflows/android-java.yml
```

The Java workflow is the NativeFlexi Android Java workflow. It uses JDK 17, Android SDK packages, Gradle, and uploads the debug APK as a GitHub Actions artifact.

## Kotlin Selection

When Kotlin is selected, MacoraOne uses:

```text
NativeFlexi-kotlin/
.github/workflows/android-kotlin.yml
```

The Kotlin workflow is the NativeFlexi Android Kotlin workflow. It uses JDK 17, Android SDK packages, Gradle, and uploads the debug APK as a GitHub Actions artifact.

The generated application does not contain both language templates. The opposite language directory is removed during creation.

## `one push AppName reponame`

After editing the generated Android project, push it with:

```text
one push AppName reponame
```

For example:

```text
one push WeatherApp WeatherApp
```

or:

```text
one push WeatherApp dominexmacedon-docs/WeatherApp
```

MacoraOne finds:

```text
~/.macoraone/projects/WeatherApp/
```

It uses the language marker created during project generation to retain the Java/Kotlin selection and force-pushes the local project tree to the requested `main` branch.

This is intentionally an override operation: the target repository becomes the current local application tree rather than accumulating an unrelated second Android template.

## `one build AppName`

Build the generated Android application through its selected GitHub Actions workflow:

```text
one build WeatherApp
```

MacoraOne reads the stored Java/Kotlin selection and chooses:

```text
Java   -> .github/workflows/android-java.yml
Kotlin -> .github/workflows/android-kotlin.yml
```

It then:

```text
1. Dispatches the selected workflow with GitHub CLI.
2. Finds the new workflow run.
3. Follows the GitHub Actions logs in the terminal.
4. Uses the workflow exit status to determine success or failure.
5. Prints the GitHub Actions artifact page URL after a successful build.
6. Opens the artifact page automatically in the default browser.
```

The artifact page is the GitHub Actions artifact location for the completed workflow run. The actual APK is uploaded by the NativeFlexi Android workflow.

## Custom Terminal Output

MacoraOne provides concise terminal status messages with ANSI colors, for example:

```text
[MacoraOne] Dispatching android-kotlin.yml for dominexmacedon-docs/WeatherApp.
> gh workflow run 'android-kotlin.yml' ...
[MacoraOne] Following GitHub Actions build logs for run 123456789.
[OK] Android build completed successfully.
[MacoraOne] Artifact URL: https://github.com/dominexmacedon-docs/WeatherApp/actions/runs/123456789/artifacts
```

The GitHub Actions logs themselves remain visible because MacoraOne follows the actual workflow run rather than hiding the build behind a separate build system.

## NativeFlexi Android Integration

NativeFlexi Android is the final conventional Android template used by MacoraOne.

Its repository provides:

```text
NativeFlexi-android/
├── NativeFlexi-java/
├── NativeFlexi-kotlin/
└── .github/workflows/
    ├── android-java.yml
    └── android-kotlin.yml
```

The Java and Kotlin projects use ordinary Android files such as:

```text
AndroidManifest.xml
build.gradle
build.gradle.kts
settings.gradle
settings.gradle.kts
*.java
*.kt
*.xml
```

Developers can therefore continue normal Android development after `one create` without learning another project language.

## Ecosystem Repositories

### greenServeFE

```text
https://github.com/dominexmacedon-docs/greenServeFE-build
https://github.com/dominexmacedon-docs/greenServeFE-
```

The build repository uses a C11 Makefile and compiles the runtime from `main.c`, lexer, parser, value, bytecode, compiler, and VM components. MacoraOne installs the build source and invokes its existing Makefile rather than reproducing the VM build logic. 

### CMLL

```text
https://github.com/dominexmacedon-docs/CMLL-build
https://github.com/dominexmacedon-docs/CMLL
```

The CMLL build repository contains the C11 command-line language implementation, including `common.c/.h`, `lexer.c/.h`, `parser.c/.h`, `runtime.c/.h`, and `main.c`. Its existing GitHub workflow compiles the native Linux executable. MacoraOne uses the same C source/build model when refreshing the local component.

### NativeFlexi

```text
https://github.com/dominexmacedon-docs/NativeFlexi
https://github.com/dominexmacedon-docs/NativeFlexi-android
```

NativeFlexi Android is the Android application template used by `one create`, `one push`, and `one build`.

## Repository Source

MacoraOne itself is maintained here:

```text
https://github.com/dominexmacedon-docs/MarocaOne-build
```

The Linux executable is built from:

```text
main.c
macoraone.h
```

and the project is compiled with standard C11 tooling through `Makefile` and `.github/workflows/build.yml`.

## Typical Workflow

A developer can use the complete flow as follows:

```text
sudo make install
one install
one create MyAndroidApp
```

Choose:

```text
Kotlin based
```

or:

```text
Java based
```

Then edit:

```text
~/.macoraone/projects/MyAndroidApp/
```

Push the edited project:

```text
one push MyAndroidApp MyAndroidApp
```

Run the selected Android GitHub Actions workflow:

```text
one build MyAndroidApp
```

After the workflow succeeds, MacoraOne prints and opens the artifact page.

## Architecture

```text
                         MacoraOne
                             |
       +---------------------+---------------------+
       |                     |                     |
   Install                  Create                Build
       |                     |                     |
       v                     v                     v
  Ecosystem repos       GitHub auth          GitHub Actions
       |                     |                     |
       +----------+----------+                     |
                  |                                |
          NativeFlexi Android                     |
                  |                                |
          +-------+-------+                        |
          |               |                        |
        Java            Kotlin                     |
          |               |                        |
          +-------+-------+                        |
                  |                                |
                  +------------> APK artifact <----+
```

## License

See the repository license for the licensing terms of MacoraOne. The projects managed by MacoraOne have their own repositories and licenses; consult each project before redistributing its source or generated software.
