idf_component_register(SRCS 
    "main.cpp"
    "system/Button.cpp"
    "system/File.cpp"
    "system/SDManager.cpp"
    "system/SPI.cpp"
    "book/Book.cpp"
    "book/PageManager.cpp"
    "ui/HebrewHelper.cpp"
    "ui/Display.cpp"
    "ui/TextBox.cpp"
    "states/ProgramState.cpp"
    "states/BookState.cpp"
    
    INCLUDE_DIRS "." "system" "ui" "book"
    REQUIRES Adafruit-GFX esp_driver_spi fatfs sdmmc
    PRIV_REQUIRES esp_timer
)
