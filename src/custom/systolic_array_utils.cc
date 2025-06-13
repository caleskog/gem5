#include "systolic_array.hh"
#include "systolic_array_utils.hh"
#include "utils_print.hh"

namespace pr = myutils::printing;
namespace sa = myutils::sa;

namespace myutils::printing::detail
{
    int precision = 0;
}
sa::PrintTile::PrintTile(gem5::SATile* tile, int precision, int padding)
    : tile(tile)
{
    inWaitingMemory = new f64[KERNEL_DIM * KERNEL_DIM];
    for (int i = 0; i < KERNEL_DIM * KERNEL_DIM; i++)
    {
        inWaitingMemory[i] = tile->inWaitingMemory[i];
    }
    pr::detail::precision = precision;
    inw_max_num_chars_cols =
        pr::getMaxPadding(inWaitingMemory, KERNEL_DIM, KERNEL_DIM);
    print_max_padding = padding;
}

void sa::PrintTile::print_top()
{
    int     padding = print_max_padding;
    size_t* inw_num_chars =
        pr::getMaxPadding(tile->inTopWaitingMemory, KERNEL_DIM, KERNEL_DIM);
    for (int i = 0; i < KERNEL_DIM - 1; i++)
    {
        padding += inw_max_num_chars_cols[i] + 1;
    }
    du_pprintcp_n(BLUE, padding, pr::ALIGNMENT::RIGHT, "");
    du_pprintc(BLUE, "Top FIFO:");
    for (int j = 0; j < KERNEL_DIM; j++)
    {
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            f64 cell = mem2d(tile->inTopWaitingMemory, KERNEL_DIM, j, i);
            if (i == 0)
            {
                du_pprintcp_n(GRAY, padding, pr::ALIGNMENT::RIGHT, "");
            }
            if (i >= MAX_ROWS - 1 - j)
            {
                du_pprintcnzp_n(GREEN, GRAY, inw_num_chars[i],
                                pr::ALIGNMENT::RIGHT, cell);
            }
            else
            {
                du_pprintcnzp_n(GREEN, GRAY, inw_num_chars[i],
                                pr::ALIGNMENT::RIGHT, "");
            }
            if (i != KERNEL_DIM - 1)
            {
                if (i >= MAX_ROWS - 1 - j)
                {
                    du_pprintc_n(GRAY, ";");
                }
                else
                {
                    du_pprintc_n(GRAY, " ");
                }
            }
        }
        std::cout << std::endl;
    }
    du_pprintcp_n(BLUE, padding, pr::ALIGNMENT::RIGHT, "");
    for (int i = 0; i < KERNEL_DIM - 1; i++)
    {
        for (int p = 0; p < inw_num_chars[i]; p++)
        {
            du_pprintc_n(GRAY, "-");
        }
        if (i != KERNEL_DIM - 1)
        {
            du_pprintc_n(GRAY, "-");
        }
    }
    std::cout << std::endl;
    delete[] inw_num_chars;
}

void sa::PrintTile::print_fifo_tile()
{
    size_t* out_num_chars =
        pr::getMaxPadding(tile->outputMemory, KERNEL_DIM, KERNEL_DIM + 1);
    size_t* in_num_chars =
        pr::getMaxPadding(tile->inputMemory, KERNEL_DIM, KERNEL_DIM);
    size_t* w_num_chars =
        pr::getMaxPadding(tile->weights, KERNEL_DIM, KERNEL_DIM);
    for (int i = 0; i < KERNEL_DIM - 1; i++)
    {
        print_max_padding += inw_max_num_chars_cols[i] + 1;
    }
    du_pprintcp_n(BLUE, print_max_padding, pr::ALIGNMENT::LEFT, "In FIFO:");
    du_pprintc(BLUE, "Systolic Array:");
    for (int i = 0; i <= KERNEL_DIM * KERNEL_DIM - 1; i++)
    {
        int x = i % KERNEL_DIM;
        int y = i / KERNEL_DIM;
        // Print the inWaitingMemory to the left of the output
        if (x == 0)
        {
            for (int j = 0; j < KERNEL_DIM - 1; j++)
            {
                f64 cell = mem2d(inWaitingMemory, KERNEL_DIM, y, j);
                // Only print if the cell is below the diagonal of the
                // waiting memory (the bottom right triangle)
                if (KERNEL_DIM - 1 - y <= j)
                {
                    du_pprintcnzp_n(RED, GRAY, inw_max_num_chars_cols[j],
                                    pr::ALIGNMENT::RIGHT, cell);
                }
                else
                {
                    du_pprintcp_n(GRAY, inw_max_num_chars_cols[j],
                                  pr::ALIGNMENT::RIGHT, "");
                }
                if (j != KERNEL_DIM - 2)
                {
                    if (KERNEL_DIM - 1 - y <= j)
                    {
                        du_pprintc_n(GRAY, ";");
                    }
                    else
                    {
                        du_pprintc_n(GRAY, " ");
                    }
                }
                else
                {
                    du_pprintc_n(GRAY, "| |");
                }
            }
        }
        f64  cell     = tile->outputMemory[i];
        f64  icell    = tile->inputMemory[i];
        f64  wcell    = tile->weights[i];
        bool set_blue = false;
        for (auto& wavefront : tile->wavefront)
        {
            if (y == wavefront - x)
            {
                du_pprintcp_n(BLUE, out_num_chars[x], pr::ALIGNMENT::RIGHT,
                              cell);
                set_blue = true;
            }
        }
        if (!set_blue)
        {
            du_pprintcnzp_n(YELLOW, GRAY, out_num_chars[x],
                            pr::ALIGNMENT::RIGHT, cell);
        }
        du_pprintc_n(GRAY, "(");
        du_pprintcnzp_n(RED, GRAY, in_num_chars[x], pr::ALIGNMENT::RIGHT,
                        icell);
        du_pprintc_n(GRAY, "*");
        du_pprintcnzp_n(GREEN, GRAY, w_num_chars[x], pr::ALIGNMENT::RIGHT,
                        wcell);
        du_pprintc_n(GRAY, ")");
        if (x != KERNEL_DIM - 1)
        {
            du_pprintc_n(GRAY, ";");
        }
        else
        {
            du_pprintc(GRAY, "");
        }
    }
    delete[] out_num_chars;
    delete[] in_num_chars;
    delete[] w_num_chars;
}

void sa::PrintTile::print_out()
{
    size_t* outw_num_chars =
        pr::getMaxPadding(tile->outWaitingMemory, KERNEL_DIM, KERNEL_DIM + 2);
    du_pprintcp_n(BLUE, print_max_padding, pr::ALIGNMENT::LEFT, "");
    du_pprintc(BLUE, "Out FIFO:");
    for (int j = 0; j < KERNEL_DIM + 2; j++)
    {
        for (int i = 0; i < KERNEL_DIM; i++)
        {
            f64 cell = mem2d(tile->outWaitingMemory, KERNEL_DIM, j, i);
            if (i == 0)
            {
                du_pprintcp_n(GRAY, print_max_padding, pr::ALIGNMENT::RIGHT,
                              "");
            }
            if (KERNEL_DIM + 1 - j >= i)
            {
                if (j == 0)
                {
                    du_pprintcnzp_n(BLUE, GRAY, outw_num_chars[i],
                                    pr::ALIGNMENT::RIGHT, cell);
                }
                else
                {
                    du_pprintcnzp_n(YELLOW, GRAY, outw_num_chars[i],
                                    pr::ALIGNMENT::RIGHT, cell);
                }
            }
            if (i != KERNEL_DIM - 1)
            {
                if (KERNEL_DIM + 1 - j > i)
                {
                    du_pprintc_n(GRAY, ";");
                }
                else
                {
                    du_pprintc_n(GRAY, " ");
                }
            }
        }
        std::cout << std::endl;
    }
    delete[] outw_num_chars;
}
