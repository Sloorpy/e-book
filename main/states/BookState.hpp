#pragma once
#include "ProgramState.hpp"

class BookState final : public ProgramState
{
public:
    explicit BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<ProgramState> prev_state);
    explicit BookState(const std::string_view& book_name, std::shared_ptr<SDManager> sd, std::unique_ptr<Display> display);
    ~BookState() override = default;

public:
    virtual void main() override;
    virtual void on_click() override;
    virtual void on_double_click() override;
    virtual void on_hold() override;   
    
private:
    std::unique_ptr<Book> create_book(const std::string_view& book_name, std::shared_ptr<SDManager> sd);

private:
    std::unique_ptr<Book> _book;
};

