idf_component_register(SRCS 
    "main.cpp"
    "system/Button.cpp"
    "system/File.cpp"
    "system/SDManager.cpp"
    "system/SPI.cpp"
    "book/Book.cpp"
    "book/PageManager.cpp"
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
    "states/ProgramState.cpp"
    "states/BookState.cpp"
    
    INCLUDE_DIRS "." "system" "ui" "book"
    REQUIRES Adafruit-GFX esp_driver_spi fatfs sdmmc
    PRIV_REQUIRES esp_timer
)
