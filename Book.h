// vim:sw=4:nu:expandtab:tabstop=4:ai

#pragma once


#include "Enums.h"
#include <map>
#include <iostream>
#include <string>
#include "Order.h"
#include <boost/unordered_map.hpp>
#include <boost/intrusive/list.hpp>

class Book
{
    using BaseList = boost::intrusive::list< Order, boost::intrusive::constant_time_size< false > >;

    std::map< Price, BaseList > bids, offers;
    boost::unordered_map< OrderID, Order* > allOrders;
    std::string name = "undef";
    Price bestBidPrice = NAN;
    Price bestOfferPrice = NAN;


public:
    Book() {}
    Book( const std::string& n ) : name(n) {}

    void printBids() {
        return;
        /*std::cout << "*** Bids ***" << std::endl;
        for( auto it = bids.begin(); it != bids.end(); it++ )
        {
            BaseList& bl = it->second;
            for( auto o : bl )
                std::cout << "$" << it->first << ": " << o.print() << std::endl;
        }
        */
    }

    Price getBestBidPrice() const   { return bestBidPrice; } 
    Price getBestOfferPrice() const { return bestOfferPrice; } 

    std::list< Order* > getOrdersInRange( Side side, Price lower, Price upper ) {
        std::list< Order* > orders;
        std::map< Price, BaseList >* ol = &bids;
        if( side == Side::Offer )
            ol = &offers;

        auto lit = ol->lower_bound( lower );
        auto uit = ol->upper_bound( upper );
        for( ; lit != uit; lit++ ) {
            BaseList& ol = lit->second;
            for( Order& o : ol ) {
                orders.push_back( &o );
            }

        }
        return orders;
    }

    int onEvent( OrderModify ev ) {
        auto ait = allOrders.find( ev.getOrderId() );
        if( unlikely( ait == allOrders.end() ) ) {
            //LE << "Order missing (cancelled?): " << ev.getOrderId() << std::endl;
            return 1;
        }

        if( unlikely( ev.getNewQty() <= 0 ) ) {  
            //LE << "Order qty negative: " << ev.getNewQty() << std::endl;
            return 2;
        }

        Order* ord = ait->second; 
        if( !ord ) {
            //LE << "Order is null: " << ev.getOrderId() << std::endl;
            return 2;
        }

        if( ev.getNewPrice() == ord->getPrice() ) { // Just price changes
            //LI << "Updating order qty to: " << ev.getNewQty() << std::endl;
            ord->setQty( ev.getNewQty() );  
            return 0;
        }

        if( ord->getSide() == Side::Bid && ev.getNewPrice() >= getBestOfferPrice() )
            return 4;
        if( ord->getSide() == Side::Offer && ev.getNewPrice() <= getBestBidPrice() )
            return 4;


        ord->unlink(); 
        Price priorPrice = ord->getPrice();

        ord->setPrice( ev.getNewPrice() );
        ord->setQty(   ev.getNewQty() );
        OrderNew on( ev.getOrderId(), ev.getNewPrice() );

        if( ord->getSide() == Side::Bid ) {
            cleanLevel( bids, priorPrice );
            processNewOrder( bids, on, ord );

            if( ev.getNewPrice() != getBestBidPrice() )
                updateBestBidPrice();

        } else {
            cleanLevel( offers, priorPrice );
            processNewOrder( offers, on, ord );

            if( ev.getNewPrice() != getBestOfferPrice() )
                updateBestOfferPrice();
        }


        printBids();
        return 0; 
    } 

    int onEvent( OrderCancel ev ) 
    {
        auto ait = allOrders.find( ev.getOrderId() );
        if( unlikely( ait == allOrders.end() ) ) {
            //LE << "Order already cancelled: " << ev.getOrderId() << std::endl;
            return 1;
        }

        Order* ord = ait->second;
        if( !ord ) {
            //LE << "Order is null: " << ev.getOrderId() << std::endl;
            return 2;
        }

        ord->unlink(); // benefit of intrusive list.

        if( ord->getSide() == Side::Bid )
        {
            cleanLevel( bids, ord->getPrice() );
            if( ord->getPrice() == getBestBidPrice() )
                updateBestBidPrice();
        } else {
            cleanLevel( offers, ord->getPrice() );
            if( ord->getPrice() == getBestOfferPrice() )
                updateBestOfferPrice();
        }

        allOrders.erase( ait );
        delete ord;

        printBids();

        return 0;
    } 

    int onEvent( OrderNew& ev ) {
        if( unlikely( allOrders.find( ev.getOrderId() ) != allOrders.end() ) ) {
            //LE << "OrderID already exists: " << ev.getOrderId() << std::endl;
            return 1;
        }

        if( unlikely( ev.getQty() <= 0 ) ) {  
            //LE << "Order qty negative: " << ev.getQty() << std::endl;
            return 2;
        }

        if( ev.getSide() == Side::Bid )
        {
            if( !std::isnan( getBestOfferPrice() ) && ev.getPrice() >= getBestOfferPrice() ) {
                //LE << "Order price higher than best offer: " << ev.getPrice() << std::endl;
                return 4;
            }

            processNewOrder( bids, ev );

            if( ev.getPrice() > getBestBidPrice() || std::isnan( getBestBidPrice() ) )
                bestBidPrice = ev.getPrice();

        } else if( ev.getSide() == Side::Offer ) {

            if( !std::isnan( getBestBidPrice() ) && ev.getPrice() <= getBestBidPrice() )
            {
                //LE << "Order price higher than best offer: " << ev.getPrice() << std::endl;
                return 4;
            }

            processNewOrder( offers, ev );

            if( ev.getPrice() < getBestOfferPrice() || std::isnan( getBestOfferPrice() ) )
                bestOfferPrice = ev.getPrice();

        } else if( ev.getSide() == Side::Undef ) {
            //LE << "Order side undefined.\n";
            return 3;
        }

        printBids();
        return 0;
    }

private:

    void updateBestOfferPrice() { 
        if( unlikely( offers.empty() ) ) { bestOfferPrice = NAN; return; }
        auto oit = offers.begin();
        BaseList& bl = oit->second;
        bestOfferPrice = bl.begin()->getPrice();
        // LI << "Updated best offer price:" << bestOfferPrice << "\n";
    }

    void updateBestBidPrice() { 
        if( unlikely( bids.empty() ) ) { bestBidPrice = NAN; return; }
        auto bit = bids.rbegin();
        BaseList& bl = bit->second;
        bestBidPrice = bl.begin()->getPrice();
    }

    void cleanLevel( std::map< Price, BaseList >& ol, Price p ) {
        auto bit = ol.find( p );
        if( bit == ol.end() ) return;

        BaseList& bl = bit->second;
        if( bl.empty() )
        {
          //LI << "Price " << p << " has no more orders - removing from map." << std::endl;
          ol.erase( bit ); 
        }
    }

    void processNewOrder( std::map< Price, BaseList >& ol, const OrderNew& ev, Order* n = nullptr ) {
        if( !n )
            n = new Order( ev );

        auto bit = ol.find( ev.getPrice() );
        if( bit == ol.end() ) { // No orders yet.
            //LI << "New orderlist into map.\n";

            BaseList bl;
            bl.push_back( *n );

            ol.emplace( ev.getPrice(), std::move( bl ) );

        } else { 
            //LI << "Push_back order onto existing BaseList.\n";
            bit->second.push_back( *n );
        }

        allOrders[ ev.getOrderId() ] = n;

    }
};



