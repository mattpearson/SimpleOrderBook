// vim:sw=4:nu:expandtab:tabstop=4:ai

#pragma once

#include <cmath>
#include <iostream>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

enum Side { Bid, Offer, Undef };
using OrderID = unsigned long long;
using Price = double;
using Qty = int;



