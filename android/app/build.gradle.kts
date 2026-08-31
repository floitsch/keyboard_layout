plugins {
    id("com.android.application")
}

android {
    namespace = "org.toitware.keyboardlayout"
    compileSdk = 36

    defaultConfig {
        applicationId = "org.toitware.keyboardlayout"
        minSdk = 16
        targetSdk = 36
        versionCode = 2
        versionName = "1.1"
    }

    androidResources {
        noCompress += "kcm"
    }
}
