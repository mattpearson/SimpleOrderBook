// vim:sw=4:nu:expandtab:tabstop=4:ai
#include <stdio.h>
#include <map>
#include <string>
#include <boost/intrusive/list.hpp>

#include "Enums.h"
#include "OrderEvents.h"

#pragma once


using auto_unlink_hook = boost::intrusive::list_base_hook< boost::intrusive::link_mode< boost::intrusive::auto_unlink> >;

class Order : public boost::intrusive::list_base_hook< boost::intrusive::link_mode<boost::intrusive::auto_unlink> >  
{
    OrderID orderId  = 0;
    Price   price    = NAN;
    Qty     qty      = 0;
    Side    side     = Side::Undef;
    

public:
    void unlink()     {  auto_unlink_hook::unlink();  }
    bool is_linked()  {  return auto_unlink_hook::is_linked();  }

    OrderID getOrderId() const { return orderId; }
    Price   getPrice()   const { return price; }
    Qty     getQty()     const { return qty; }
    Side    getSide()    const { return side; }

    void    setQty( Qty q )    { qty = q; } 
    void    setPrice( Price p ){ price = p; } 

    Order( OrderNew ev )
        : orderId( ev.getOrderId() )
        , price( ev.getPrice() )
        , qty( ev.getQty() )
        , side( ev.getSide() )
        {}

    std::string print() { return std::to_string( orderId ); } 

};



