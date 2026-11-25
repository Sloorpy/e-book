idf_component_register(SRCS 
    "main.cpp"
    "system/Button.cpp"
    "system/File.cpp"
    "system/SDManager.cpp"
    "hardware/SPI.cpp"
    "book/Book.cpp"
    "book/BookFs.cpp"
    "ui/Text/Legacy/BookString.cpp"
    "ui/Text/Legacy/Word.cpp"
    "ui/Text/Layout/BidiResolver.cpp"
    "ui/Text/Layout/LineBreaker.cpp"
    "ui/Text/Layout/TextLayout.cpp"
    "ui/Text/Layout/TextTokenizer.cpp"
    "ui/Text/Rendering/TextBox.cpp"
    "ui/Text/Rendering/ScopedDisplayFont.cpp"
    "ui/Text/Support/TextHelper.cpp"
    "ui/Display.cpp"
    "ui/core/CanvasRegion.cpp"
    "ui/core/HomeButton.cpp"
    "ui/ButtonsArray.cpp"
    "states/BookState.cpp"
    "states/HomeState.cpp"
    "system/PageBuffer.cpp"
    "book/PageIter.cpp"
    "hardware/AdcHandle.cpp"
    "hardware/AdcReader.cpp"
    "system/Battery.cpp"
    INCLUDE_DIRS "." "system" "ui" "book" "hardware"
    REQUIRES Adafruit-GFX esp_driver_spi fatfs sdmmc
    PRIV_REQUIRES esp_timer esp_adc
)

target_sources(${COMPONENT_LIB} PRIVATE "ui/ButtonsArray.hpp")
