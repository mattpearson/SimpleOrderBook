// vim:sw=4:nu:expandtab:tabstop=4:ai

#pragma once


#include "Enums.h"

class OrderNew
{
    OrderID orderId  = 0;
    Price   price    = NAN;
    Qty     qty      = 0;
    Side    side     = Side::Undef;

public:
    OrderNew( OrderID oid, Side s, Price p, Qty q )
        : orderId( oid )
        , price( p )
        , qty ( q )
        , side ( s )
        {}
    OrderNew( OrderID oid, Price p ) // customized for modify
        : orderId( oid )
        , price( p )
        {}

    OrderID getOrderId() const { return orderId; }
    Price   getPrice()   const { return price; }
    Qty     getQty()     const { return qty; }
    Side    getSide()    const { return side; }
};


class OrderCancel
{
    OrderID orderId  = 0;
public:
    OrderCancel( OrderID oid )
        : orderId( oid )
        {}
    OrderID getOrderId() const { return orderId; }
};


class OrderModify
{
    OrderID orderId  = 0;
    Price   newPrice = NAN;
    Qty     newQty   = 0;
public:
    OrderModify( OrderID oid, Price np, Qty nq )
        : orderId( oid )
        , newPrice( np )
        , newQty( nq )
        {}
    OrderID getOrderId()  const { return orderId; }
    Price   getNewPrice() const { return newPrice; }
    Qty     getNewQty()   const { return newQty; }
};


