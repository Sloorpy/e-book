idf_component_register(SRCS 
    "main.cpp"
    "system/Button.cpp"
    "system/File.cpp"
    "system/SDManager.cpp"
    "system/SPI.cpp"
    "book/Book.cpp"
    "book/PageManager.cpp"
    "ui/Text/BookString.cpp"
    "ui/Text/TextHelper.cpp"
    "ui/Text/Word.cpp"
    "ui/Text/TextBox.cpp"
    "ui/Display.cpp"
    "states/ProgramState.cpp"
    "states/BookState.cpp"
    
    INCLUDE_DIRS "." "system" "ui" "book"
    REQUIRES Adafruit-GFX esp_driver_spi fatfs sdmmc
    PRIV_REQUIRES esp_timer
)
