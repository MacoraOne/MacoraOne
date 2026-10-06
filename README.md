# NativeFlexi Android

## Native Android Development Template

**NativeFlexi Android** is a ready-to-use Android project template for developers who want to build native Android applications using standard **Java or Kotlin**, Android SDK tools, Gradle, XML resources, and Android Studio.

NativeFlexi does not introduce a custom programming language, custom compiler, custom UI syntax, or generated runtime.

Developers work directly with normal Android project files and can modify the application according to their own requirements.

The repository provides two independent Android templates:

```text
NativeFlexi-android
│
├── NativeFlexi-java/
│   └── Java Android application
│
├── NativeFlexi-kotlin/
│   └── Kotlin Android application
│
└── .github/
    └── workflows/
        ├── android-java.yml
        └── android-kotlin.yml
```

---

# MacoraOne

## One Technology Ecosystem. Multiple Possibilities.

**MacoraOne** is a software technology ecosystem focused on programming languages, developer tools, server-side development, command-line utilities, Android development, runtime systems, virtual machines, and resources for modern software engineering.

Our goal is to bring independent developer projects together under one unified technology ecosystem, making software development more accessible, practical, and extensible.

From programming languages and runtime systems to server infrastructure, command-line tools, Android development templates, and AI-oriented datasets, MacoraOne brings together technologies designed to support developers, learners, and technology builders.

**NativeFlexi Android is part of the MacoraOne ecosystem.**

---

## About MacoraOne

MacoraOne is the unified technology identity for a collection of software development projects created by **Dominex Macedon**.

The ecosystem includes programming languages, runtime components, virtual machines, server development utilities, command-line tools, Android development resources, developer tooling, and data resources for AI-related research and development.

Each project serves a different purpose while contributing to a broader vision: building practical software technologies that developers can explore, use, modify, and extend.

For information about the creator and developer behind MacoraOne, see [`ABOUT_DEVELOPER.md`](ABOUT_DEVELOPER.md).

### Our Focus

* Programming language development
* Server-side programming
* Android development
* Native Java and Kotlin development
* Developer tools and runtime systems
* Virtual-machine and bytecode technologies
* Command-line software
* AI-oriented programming language datasets
* Open-source software experimentation
* Accessible developer experiences

---

# NativeFlexi Android

NativeFlexi Android provides a conventional Android project structure that developers can clone, fork, modify, and use as the foundation for their own applications.

There are no special NativeFlexi source files that developers need to learn.

Developers use:

* Java or Kotlin
* Android SDK
* Android Studio
* Gradle
* Gradle Kotlin DSL
* XML resources
* AndroidManifest.xml
* Android application components
* Android libraries and dependencies
* Git and GitHub

The repository is intended to provide a clean starting point rather than hide Android development behind another abstraction.

---

# Choose Java or Kotlin

NativeFlexi Android provides two separate templates.

## Java

Use:

```text
NativeFlexi-java/
```

The Java project uses:

```text
build.gradle
settings.gradle
*.java
```

The main application source is:

```text
NativeFlexi-java/app/src/main/java/com/nativeflexi/MainActivity.java
```

Build it with:

```text
cd NativeFlexi-java
gradle assembleDebug --no-daemon
```

---

## Kotlin

Use:

```text
NativeFlexi-kotlin/
```

The Kotlin project uses:

```text
build.gradle.kts
settings.gradle.kts
*.kt
```

The main application source is:

```text
NativeFlexi-kotlin/app/src/main/java/com/nativeflexi/MainActivity.kt
```

Build it with:

```text
cd NativeFlexi-kotlin
gradle assembleDebug --no-daemon
```

Choose the language you want to use and develop that project independently.

---

# Clone the Repository

Developers can clone the repository directly:

```text
git clone https://github.com/dominexmacedon-docs/NativeFlexi-android.git
```

Then enter the repository:

```text
cd NativeFlexi-android
```

Choose either:

```text
NativeFlexi-java
```

or:

```text
NativeFlexi-kotlin
```

Open the selected directory in Android Studio.

---

# Fork the Repository

NativeFlexi Android is designed to be forkable.

A developer can create their own GitHub fork and use the fork as the starting point for a new Android application or development project.

Typical workflow:

```text
NativeFlexi-android
        │
        └── Fork
             │
             ▼
      Developer's GitHub repository
             │
             ├── Modify Java/Kotlin
             ├── Modify XML resources
             ├── Modify Manifest
             ├── Modify Gradle
             ├── Add dependencies
             └── Build application
```

After creating a fork, clone the developer's fork locally:

```text
git clone https://github.com/YOUR-USERNAME/NativeFlexi-android.git
```

Then:

```text
cd NativeFlexi-android
```

The developer can make their changes and push them to their own repository.

For example:

```text
git add .
git commit -m "Customize Android application"
git push
```

This allows developers to maintain their own version without modifying the original MacoraOne NativeFlexi repository.

---

# What Developers Should Edit

NativeFlexi is intentionally based on normal Android project files.

Developers can directly edit the following.

## Java or Kotlin Source

Java:

```text
NativeFlexi-java/app/src/main/java/
```

Kotlin:

```text
NativeFlexi-kotlin/app/src/main/java/
```

Developers can add:

* Activities
* Fragments
* Services
* Broadcast receivers
* ViewModels
* Adapters
* Data classes
* Utility classes
* Networking code
* Application logic

---

# AndroidManifest.xml

The application manifest is located at:

```text
NativeFlexi-java/app/src/main/AndroidManifest.xml
```

or:

```text
NativeFlexi-kotlin/app/src/main/AndroidManifest.xml
```

Developers can modify the manifest to add application components and configuration.

Examples include:

* Permissions
* Activities
* Services
* Broadcast receivers
* Content providers
* Intent filters
* Deep links
* Application configuration
* Export settings

For permissions that require user approval at runtime, developers must also implement the appropriate runtime permission handling in Java or Kotlin.

---

# Android Resources

Resources are located under:

```text
app/src/main/res/
```

The template includes standard Android resources such as:

```text
res/
├── layout/
├── values/
├── drawable/
├── mipmap/
├── menu/
└── xml/
```

Developers can add or modify resources according to their application.

## Layouts

XML layouts are stored in:

```text
app/src/main/res/layout/
```

For example:

```text
activity_main.xml
```

Developers can replace the sample interface with their own Android UI.

## Strings

String resources are stored in:

```text
app/src/main/res/values/strings.xml
```

## Colors

Colors are stored in:

```text
app/src/main/res/values/colors.xml
```

## Themes

Themes are stored in:

```text
app/src/main/res/values/themes.xml
```

Developers can modify these files or introduce additional Android resource configurations.

---

# Gradle Configuration

NativeFlexi uses standard Android Gradle configuration.

Java:

```text
NativeFlexi-java/build.gradle
NativeFlexi-java/settings.gradle
NativeFlexi-java/app/build.gradle
```

Kotlin:

```text
NativeFlexi-kotlin/build.gradle.kts
NativeFlexi-kotlin/settings.gradle.kts
NativeFlexi-kotlin/app/build.gradle.kts
```

Developers can modify the Gradle configuration to customize their application.

---

# Application Identity

The application namespace and package configuration can be changed in the module Gradle file.

For example:

```text
namespace = "com.nativeflexi"
```

The application ID is also configurable:

```text
applicationId = "com.nativeflexi"
```

Developers should change these values when creating a separate application.

For example:

```text
com.example.myapplication
```

The Java or Kotlin package declarations should be updated consistently with the selected namespace and source structure.

---

# Android SDK Configuration

The template specifies Android SDK versions in the Gradle configuration.

Developers can change:

```text
compileSdk
minSdk
targetSdk
```

according to their application's requirements and the Android SDK versions installed in their development environment.

For example:

```text
compileSdk = 35
minSdk = 24
targetSdk = 35
```

These values are examples used by the template and can be changed when necessary.

---

# Dependencies

Application dependencies are declared in:

```text
app/build.gradle
```

or:

```text
app/build.gradle.kts
```

For example:

```text
implementation(...)
```

Developers can add Android libraries and other compatible dependencies as required.

When adding a dependency, developers should verify:

* Android compatibility
* Version compatibility
* Minimum SDK requirements
* Licensing
* Repository availability
* Transitive dependencies

---

# Build Types

NativeFlexi includes standard Android build types.

The default configuration includes:

```text
debug
release
```

Developers can customize build types for their application.

Release builds can be configured for:

* Optimization
* Shrinking
* Resource optimization
* ProGuard/R8 rules
* Signing
* Production configuration

Sensitive signing credentials should never be committed to the repository.

---

# Android Studio

The recommended development environment is **Android Studio**.

Developers can open either project:

```text
NativeFlexi-java/
```

or:

```text
NativeFlexi-kotlin/
```

in Android Studio.

Android Studio can synchronize the Gradle project and provide:

* Code editing
* Android SDK management
* Emulator support
* Device deployment
* Debugging
* Logcat
* Resource editing
* Gradle integration
* APK generation

---

# Local Build

## Java

```text
cd NativeFlexi-java
gradle assembleDebug --no-daemon
```

The debug APK is generated under:

```text
app/build/outputs/apk/debug/
```

## Kotlin

```text
cd NativeFlexi-kotlin
gradle assembleDebug --no-daemon
```

The debug APK is generated under:

```text
app/build/outputs/apk/debug/
```

---

# GitHub Actions

NativeFlexi provides separate GitHub Actions workflows for the Java and Kotlin templates.

```text
.github/workflows/
├── android-java.yml
└── android-kotlin.yml
```

Developers can manually choose the workflow corresponding to the project they are developing.

## Java Workflow

```text
android-java.yml
```

Builds:

```text
NativeFlexi-java
```

## Kotlin Workflow

```text
android-kotlin.yml
```

Builds:

```text
NativeFlexi-kotlin
```

The workflows install the required Android SDK packages, build the debug APK, and upload the resulting APK as a GitHub Actions artifact.

Developers can modify these workflows when their own project requires additional SDK packages, tests, signing configuration, or deployment steps.

---

# No Custom NativeFlexi Language

NativeFlexi Android does **not** require:

```text
.nfx
```

files.

There is no NativeFlexi compiler.

There is no NativeFlexi interpreter.

There is no generated Android runtime.

There is no custom UI language.

There is no custom application syntax.

Developers directly write normal:

```text
.java
.kt
.xml
.gradle
.gradle.kts
```

files.

This keeps the project compatible with conventional Android development workflows.

---

# Recommended Development Workflow

After cloning or forking the repository:

```text
1. Choose Java or Kotlin
2. Open the selected project in Android Studio
3. Change the application namespace
4. Change the application ID
5. Update the application name
6. Modify MainActivity
7. Create your application UI
8. Add required resources
9. Configure AndroidManifest.xml
10. Add required dependencies
11. Configure SDK versions
12. Configure build types
13. Test on an emulator or physical device
14. Build the debug APK
15. Configure release signing when ready
16. Commit your application
17. Push your changes
```

---

# Repository Structure

```text
NativeFlexi-android/
│
├── NativeFlexi-java/
│   ├── app/
│   │   ├── src/
│   │   │   └── main/
│   │   │       ├── AndroidManifest.xml
│   │   │       ├── java/
│   │   │       │   └── com/nativeflexi/
│   │   │       │       └── MainActivity.java
│   │   │       └── res/
│   │   │           ├── layout/
│   │   │           └── values/
│   │   ├── build.gradle
│   │   └── proguard-rules.pro
│   │
│   ├── build.gradle
│   ├── gradle.properties
│   └── settings.gradle
│
├── NativeFlexi-kotlin/
│   ├── app/
│   │   ├── src/
│   │   │   └── main/
│   │   │       ├── AndroidManifest.xml
│   │   │       ├── java/
│   │   │       │   └── com/nativeflexi/
│   │   │       │       └── MainActivity.kt
│   │   │       └── res/
│   │   │           ├── layout/
│   │   │           └── values/
│   │   ├── build.gradle.kts
│   │   └── proguard-rules.pro
│   │
│   ├── build.gradle.kts
│   ├── gradle.properties
│   └── settings.gradle.kts
│
├── .github/
│   └── workflows/
│       ├── android-java.yml
│       └── android-kotlin.yml
│
├── .gitignore
├── LICENSE
└── README.md
```

---

# MacoraOne Ecosystem

```text
MacoraOne
│
├── Programming Languages
│   ├── Puma
│   │   ├── Puma Language
│   │   └── Puma Runtime
│   │
│   ├── greenServeFE-
│   │   ├── Interpreter
│   │   ├── Compiler
│   │   ├── Bytecode
│   │   ├── Virtual Machine
│   │   └── Native Modules
│   │
│   └── CMLL
│       └── Command-Line Programming
│
├── Android Development
│   └── NativeFlexi Android
│       ├── Java Template
│       ├── Kotlin Template
│       ├── Android SDK
│       ├── Gradle
│       └── GitHub Actions
│
├── AI & Data
│   └── Puma Data for AI Models
│
└── Developer Ecosystem
    ├── Language Tooling
    ├── Runtime Systems
    ├── Virtual Machines
    ├── Android Development
    └── Open-Source Projects
```

---

# MacoraOne Projects

MacoraOne brings together the following projects.

## 1. Puma Programming Language

**A programming language designed for server-related development and beginner-friendly programming.**

Puma is a programming language project with a custom runtime and `.pulsar` source-file extension.

The project documentation describes language features such as variables, functions, control flow, asynchronous tasks, web server utilities, and networking-related functionality.

**Repository:**

https://github.com/dominexmacedon-docs/puma

**Language and runtime repository:**

https://github.com/dominexmacedon-docs/puma-programming-language

**Key areas:**

* Custom programming language syntax
* `.pulsar` source files
* Runtime and execution environment
* Server-side programming
* Functions and control flow
* Web server development

---

## 2. Puma Data for AI Models

**A dedicated repository for Puma-related data and resources for AI model development.**

This project is part of the Puma ecosystem and supports work involving programming language data and AI-oriented development.

**Repository:**

https://github.com/dominexmacedon-docs/puma-data-for-ai-models

**Project area:**

* Programming language datasets
* AI-oriented resources
* Puma language data
* Developer and research workflows

Refer to the repository documentation for the exact dataset structure, licensing, and supported use cases.

---

## 3. greenServeFE-

**A fast-execution programming language runtime and virtual-machine-based development platform.**

greenServeFE- is focused on fast program execution through a custom interpreter, compiler, bytecode system, virtual machine, and native module architecture.

**Project repository:**

https://github.com/dominexmacedon-docs/greenServeFE-

**Project area:**

* Programming language runtime development
* Interpreter and compiler development
* Bytecode execution
* Virtual-machine architecture
* Native C modules
* Server-side development
* Runtime and developer tooling
* Fast program execution

---

## 4. CMLL

**Command Line Language — a programming language designed for command-line execution and scripting.**

CMLL is a command-line language project focused on multi-line execution, scripting, and command-line workflows.

**Repository:**

https://github.com/dominexmacedon-docs/CMLL

**Project area:**

* Command-line programming
* Script execution
* Multi-line language syntax
* Developer utilities
* CLI-oriented workflows

---

## 5. NativeFlexi Android

**A native Android development template for Java and Kotlin developers.**

NativeFlexi Android provides conventional Android project structures that developers can clone or fork and use as the foundation for their own applications.

**Repository:**

https://github.com/dominexmacedon-docs/NativeFlexi-android

**Project area:**

* Android development
* Java Android applications
* Kotlin Android applications
* Android SDK
* Gradle
* XML resources
* Android Studio
* GitHub Actions
* Open-source development templates

---

# Why MacoraOne?

Software development is built on different layers of technology.

Programming languages provide the foundation. Runtime systems execute applications. Virtual machines provide execution environments. Server tools support backend development. Command-line utilities simplify workflows. Android development provides mobile application capabilities. Data resources support experimentation and research.

MacoraOne brings these areas together under one recognizable identity.

The ecosystem is designed around three principles.

## Accessibility

Create software projects that are approachable for developers at different experience levels.

## Practical Development

Focus on tools and programming technologies that can be explored through real development workflows.

## Continuous Innovation

Experiment with programming languages, runtimes, virtual machines, developer utilities, Android development, and new approaches to software development.

---

# Technology Areas

| Technology            | Description                                               |
| --------------------- | --------------------------------------------------------- |
| Programming Languages | Custom language development and syntax design             |
| Android Development   | Native Java and Kotlin application development            |
| Server Development    | Tools and languages for server-oriented programming       |
| Runtime Systems       | Components that execute and support programming languages |
| Virtual Machines      | Bytecode-based execution and runtime systems              |
| Command-Line Tools    | Utilities and languages for terminal workflows            |
| AI Data Resources     | Programming-related data and AI-oriented resources        |
| Developer Tooling     | Supporting tools for programming and software development |

The exact features and maturity of each project depend on its individual repository.

---

# Open Source

MacoraOne projects are developed through public GitHub repositories.

Developers can:

* Explore source code
* Clone repositories
* Fork repositories
* Review documentation
* Test available software
* Modify projects
* Build their own versions
* Contribute where the individual repository supports contributions

Project-specific licenses and contribution guidelines apply.

Before redistributing or publishing a modified project, review the license of the repository you are using.

---

# Explore the Ecosystem

## Programming Languages

* [Puma](https://github.com/dominexmacedon-docs/puma)
* [Puma Programming Language](https://github.com/dominexmacedon-docs/puma-programming-language)
* [greenServeFE-](https://github.com/dominexmacedon-docs/greenServeFE-)
* [CMLL](https://github.com/dominexmacedon-docs/CMLL)

## Android Development

* [NativeFlexi Android](https://github.com/dominexmacedon-docs/NativeFlexi-android)

## Data and Resources

* [Puma Data for AI Models](https://github.com/dominexmacedon-docs/puma-data-for-ai-models)

---

# Getting Started with MacoraOne

Choose a project based on your interests.

**Android development:**
Explore NativeFlexi Android and choose the Java or Kotlin template.

**Programming languages:**
Explore Puma, greenServeFE-, or CMLL.

**Server development:**
Explore the Puma and greenServeFE- projects and review their respective runtime and server documentation.

**AI and data:**
Explore the Puma data repository for its available resources and documentation.

**Command-line development:**
Explore CMLL and its supported scripting and execution features.

**Runtime and VM development:**
Explore greenServeFE- and its interpreter, compiler, bytecode, VM, and native-module architecture.

Each repository contains its own installation and usage instructions.

---

# Developer

**Dominex Macedon**

Creator and developer of the MacoraOne project ecosystem.

For detailed information about Dominex Macedon, see [`ABOUT_DEVELOPER.md`](ABOUT_DEVELOPER.md).

**GitHub:**

https://github.com/dominexmacedon-docs

---

# Vision

MacoraOne aims to develop a connected ecosystem of software projects that supports programming education, experimentation, and practical developer workflows.

Through programming languages, server tools, command-line utilities, virtual machines, runtime systems, Android development templates, and data resources, the platform provides a foundation for exploring different areas of software technology.

**One identity. Multiple projects. A growing technology ecosystem.**

---

# License

Licensing varies by repository.

Refer to the license file in each individual project before using, modifying, or redistributing its contents.

---

# MacoraOne

**Programming Languages • Android Development • Server Development • Developer Tools • AI Resources • Open Source**
