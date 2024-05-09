#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");
    
    const auto& cell = cells_.find(pos);

    if (cell == cells_.end()) cells_.emplace(pos, std::make_unique<Cell>(*this));
    cells_.at(pos)->Set(std::move(text));
    if(pos.row+1>=max_rows_[max_rows_.size()-1])
    {
        max_rows_.push_back(pos.row + 1);
    }
    if(pos.col+1>=max_cols_[max_cols_.size()-1])
    {
        max_cols_.push_back(pos.col + 1);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(pos.IsValid())
    {
        return GetCellPtr(pos);
    }
    else
    {
        throw InvalidPositionException{"invalid position"};
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if(pos.IsValid())
    {
        return GetCellPtr(pos);
    }
    else
    {
        throw InvalidPositionException{"invalid position"};
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    const auto& cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second != nullptr) {
        cell->second->Clear();
        if (!cell->second->IsReferenced()) {
            cell->second.reset();
        }
    }
    auto rows_it=std::find(max_rows_.begin(), max_rows_.end(), pos.row+1);
    auto cols_it=std::find(max_cols_.begin(), max_cols_.end(), pos.col+1);
    if(rows_it!=max_rows_.end())
    {
        max_rows_.erase(rows_it);
    }
    if(cols_it!=max_cols_.end())
    {
        max_cols_.erase(cols_it);
    }
}

Size Sheet::GetPrintableSize() const 
{
    return Size{max_rows_[max_rows_.size()-1], max_cols_[max_cols_.size()-1]};
}

void Sheet::PrintValues(std::ostream& output) const 
{
    auto s=GetPrintableSize();
    for(int i=0; i<s.rows; i++)
    {
        bool first=true;
        for(int j=0; j<s.cols; j++)
        {
            if(first)
            {
                first=false;
            }
            else
            {
                output<<'\t';
            }
            if(auto p=GetCell({i, j}))
            {
                auto value=p->GetValue();
                try
                {
                    double x=std::get<double>(value);
                    output<<x;
                }
                catch (std::bad_variant_access const& ex)
                {
                    try
                    {
                        std::string x=std::get<std::string>(value);
                        output<<x;
                    }
                    catch (std::bad_variant_access const& ex)
                    {
                        FormulaError x=std::get<FormulaError>(value);
                        output<<x;
                    }
                }
            }
        }
        output<<'\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const 
{
    auto s=GetPrintableSize();
    for(int i=0; i<s.rows; i++)
    {
        bool first=true;
        for(int j=0; j<s.cols; j++)
        {
            if(first)
            {
                first=false;
            }
            else
            {
                output<<'\t';
            }
            if(auto p=GetCell({i, j}))
            {
                output<<p->GetText();
            }
        }
        output<<'\n';
    }
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    const auto cell = cells_.find(pos);
    if (cell == cells_.end()) {
        return nullptr;
    }

    return cells_.at(pos).get();
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(
        static_cast<const Sheet&>(*this).GetCellPtr(pos));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}