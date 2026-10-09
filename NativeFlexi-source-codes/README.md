# NativeFlexi

NativeFlexi is a collection of normal Android application templates. It does not define a custom UI language or generate Android source.

Choose one:
- `NativeFlexi-java/` — Java + Groovy Gradle
- `NativeFlexi-kotlin/` — Kotlin + Kotlin DSL

Developers edit normal Android files manually: Java/Kotlin, XML resources, AndroidManifest.xml, Gradle files, dependencies, permissions, build types, signing and CI.

## Structure

```
NativeFlexi/
├── NativeFlexi-java/
│   ├── app/src/main/AndroidManifest.xml
│   ├── app/src/main/java/com/nativeflexi/MainActivity.java
│   ├── app/src/main/res/layout/activity_main.xml
│   ├── app/src/main/res/values/
│   ├── app/build.gradle
│   ├── build.gradle
│   ├── settings.gradle
│   └── gradle.properties
├── NativeFlexi-kotlin/
│   ├── app/src/main/AndroidManifest.xml
│   ├── app/src/main/java/com/nativeflexi/MainActivity.kt
│   ├── app/src/main/res/layout/activity_main.xml
│   ├── app/src/main/res/values/
│   ├── app/build.gradle.kts
│   ├── build.gradle.kts
│   ├── settings.gradle.kts
│   └── gradle.properties
└── .github/workflows/
    ├── android-java.yml
    └── android-kotlin.yml
```

## Choose a template

Java:
```bash
cd NativeFlexi-java
gradle assembleDebug
```

Kotlin:
```bash
cd NativeFlexi-kotlin
gradle assembleDebug
```

Open the selected folder in Android Studio. The repository root is not itself an Android project.

## Java template: what to edit

Edit `settings.gradle` to rename the project:
```gradle
rootProject.name = 'MyApplication'
include ':app'
```

Edit `app/build.gradle` for Android configuration:
```gradle
android {
    namespace 'com.example.myapplication'
    compileSdk 35

    defaultConfig {
        applicationId 'com.example.myapplication'
        minSdk 24
        targetSdk 35
        versionCode 1
        versionName '1.0'
    }
}
```

Change namespace, applicationId, SDK levels, versionCode and versionName intentionally. Add dependencies in `dependencies { }` using Groovy syntax.

The Java source is `app/src/main/java/.../MainActivity.java`. Change its `package` declaration when changing the namespace.

## Kotlin template: what to edit

Edit `settings.gradle.kts`:
```kotlin
rootProject.name = "MyApplication"
include(":app")
```

Edit `app/build.gradle.kts`:
```kotlin
android {
    namespace = "com.example.myapplication"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.example.myapplication"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
    }
}
```

Kotlin DSL is Kotlin syntax; do not paste Groovy syntax directly into `.kts` files. Add dependencies with `implementation("group:artifact:version")`.

The Kotlin source is `app/src/main/java/.../MainActivity.kt`. Change its `package` declaration when changing the namespace.

## Gradle and .kts responsibilities

Root Gradle files select Android/Kotlin plugins and repositories. The app module file controls namespace, application ID, SDK versions, dependencies, build types, Java/Kotlin compatibility, packaging and release configuration.

When changing Android Gradle Plugin, Gradle, JDK, Kotlin, compileSdk or related tooling, keep versions compatible. Use the JDK supported by the selected Android Gradle Plugin.

The Java template uses `build.gradle`, while the Kotlin template uses `build.gradle.kts`. Their configuration is intentionally equivalent but their syntax is different.

Do not commit signing passwords or keystores. Configure release signing from secure local/CI secrets.

## AndroidManifest.xml

Edit:
```
app/src/main/AndroidManifest.xml
```

Use the manifest for application metadata and Android components: activities, services, broadcast receivers, content providers, intent filters, exported state and permissions.

Example:
```xml
<uses-permission android:name="android.permission.INTERNET" />
```

Only add permissions required by the application.

For protected runtime permissions such as camera or location, manifest declaration alone is not enough. The application must check permission state, request the permission at runtime on applicable Android versions, handle the result, and only access the protected API after permission is granted. Follow current Android privacy and platform rules.

## Resources

Use normal Android resources in `app/src/main/res/`:
- `layout/` — XML layouts
- `drawable/` — images and drawables
- `mipmap/` — launcher icons
- `values/` — strings, colors, themes and other values
- `menu/`, `xml/`, `font/` — optional resource types

Edit `activity_main.xml` for the sample screen. Replace it with your own layouts or build views programmatically.

Use standard Android components such as TextView, Button, EditText, ImageView, RecyclerView, ScrollView, ConstraintLayout, fragments, services and other Android APIs. There is no NativeFlexi component syntax.

## Icons and images

Put drawable resources in `res/drawable` and launcher assets in the normal Android `mipmap` directories. Reference them with normal Android resource IDs such as `@drawable/logo` or `R.drawable.logo`.

There is no custom NativeFlexi icon syntax.

## Permissions

Common permissions include:
```xml
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.CAMERA" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

Do not request unrelated or unnecessary permissions. Some permissions are normal while others require runtime approval. Sensitive permissions can also be subject to Google Play policy.

## Build types

Both templates contain normal `debug` and `release` build types. Developers can add signing, R8/ProGuard rules, resource shrinking, build variants and packaging settings manually.

Release signing should use a secure keystore and secrets. Never place passwords in source control.

## Java/Kotlin compatibility

The templates use Java 17 compatibility. If you change Java/Kotlin versions, update the Gradle/JDK/toolchain configuration consistently and verify the Android Gradle Plugin supports the selected toolchain.

## GitHub Actions

There are separate manual workflows:
- `.github/workflows/android-java.yml`
- `.github/workflows/android-kotlin.yml`

Choose the workflow matching the template. In GitHub, open Actions, select the workflow, choose **Run workflow**, select the branch and start it.

The Java workflow enters `NativeFlexi-java` and builds `assembleDebug`. The Kotlin workflow enters `NativeFlexi-kotlin` and builds `assembleDebug`. Each uploads the resulting APK.

They are manual by design; choosing Java or Kotlin is explicit.

## Local build

Java:
```bash
cd NativeFlexi-java
gradle assembleDebug
```

Kotlin:
```bash
cd NativeFlexi-kotlin
gradle assembleDebug
```

APK output:
```
app/build/outputs/apk/debug/app-debug.apk
```

Install with standard Android tooling:
```bash
adb install app/build/outputs/apk/debug/app-debug.apk
```

## Recommended editing order

1. Choose Java or Kotlin.
2. Change project name.
3. Change namespace and package.
4. Change application ID.
5. Set compileSdk, minSdk and targetSdk.
6. Set versionCode/versionName.
7. Edit AndroidManifest.xml.
8. Add only required permissions.
9. Edit the activity.
10. Edit layouts and resources.
11. Add dependencies.
12. Configure release signing securely.
13. Build locally.
14. Run the matching manual GitHub Actions workflow.
15. Test the APK.

## Manual philosophy

Older revisions experimented with a custom `.nfx` source format, parsers and generated Android runtime code. That design has been removed.

There is intentionally no:
- `.nfx` application source
- custom UI parser
- generated Java runtime
- generated Android project
- custom icon syntax
- NativeFlexi UI compiler

NativeFlexi is now a thin, conventional Android starting point. Android Studio, Gradle, the Android SDK and developer-written Java/Kotlin are the source of truth.
