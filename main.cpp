// vim:sw=4:nu:expandtab:tabstop=4:ai
#include <iostream>

#include <cstdlib> 
#include "Enums.h"
#include "OrderEvents.h"
#include "Book.h"
#include <chrono>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Main
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( BookTest ) 


BOOST_AUTO_TEST_CASE( New )
{
    using namespace std;

    Book book("TestInst");

    OrderNew on1( 1, Side::Bid, 10, 100 );
    book.onEvent( on1 );
    OrderNew on2( 2, Side::Bid, 10, 100 );
    book.onEvent( on2 );
    OrderNew on3( 3, Side::Offer, 13, 100 );
    book.onEvent( on3 );

    BOOST_REQUIRE( book.getBestBidPrice() == 10 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 13 );

    // Make new best bid
    OrderNew on4( 4, Side::Bid, 11, 100 );
    book.onEvent( on4 );
    BOOST_REQUIRE( book.getBestBidPrice() == 11 );

    // Make sure best bid still remains 11
    OrderNew on5( 5, Side::Bid, 9, 100 );
    book.onEvent( on5 );
    BOOST_REQUIRE( book.getBestBidPrice() == 11 );

    // Test bid through best offer
    OrderNew on6( 6, Side::Bid, 14, 100 );
    BOOST_REQUIRE( book.onEvent( on6 ) == 4 );
    BOOST_REQUIRE( book.getBestBidPrice() == 11 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 13 );

    // Test offer through best bid
    OrderNew on7( 7, Side::Offer, 10, 100 );
    BOOST_REQUIRE( book.onEvent( on7 ) == 4 );
    BOOST_REQUIRE( book.getBestBidPrice() == 11 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 13 );

    // Test negative qty
    OrderNew on8( 8, Side::Offer, 10, -100 );
    BOOST_REQUIRE( book.onEvent( on8 ) > 0 );


    // Test already existing order.
    OrderNew on9a( 3, Side::Offer, 30, 100 );
    BOOST_REQUIRE( book.onEvent( on9a ) > 0 );
    OrderNew on9b( 3, Side::Bid, 30, 100 );
    BOOST_REQUIRE( book.onEvent( on9b ) > 0 );
}

BOOST_AUTO_TEST_CASE( Cancel )
{
    using namespace std;

    Book book("TestInst");

    OrderNew on1( 1, Side::Bid, 10, 100 );
    book.onEvent( on1 );
    OrderNew on2( 2, Side::Bid, 10, 100 );
    book.onEvent( on2 );
    OrderNew on3( 3, Side::Offer, 13, 100 );
    book.onEvent( on3 );

    BOOST_REQUIRE( book.getBestBidPrice() == 10 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 13 );

    OrderCancel oc1( 2 );
    book.onEvent( oc1 );
    BOOST_REQUIRE( book.getBestBidPrice() == 10 );

    OrderCancel oc2( 1 );
    BOOST_REQUIRE( book.onEvent( oc2 ) == 0 );
    BOOST_REQUIRE( std::isnan( book.getBestBidPrice() ) == true );

    BOOST_REQUIRE( book.onEvent( oc2 ) > 0 );


    OrderNew on4( 4, Side::Offer, 12, 100 );
    book.onEvent( on4 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 12 );

    OrderCancel oc4( 4 );
    BOOST_REQUIRE( book.onEvent( oc4 ) == 0 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 13 );
}

BOOST_AUTO_TEST_CASE( Modify )
{
    using namespace std;

    Book book("TestInst");

    OrderNew on1( 1, Side::Bid, 10, 100 );
    book.onEvent( on1 );
    OrderNew on2( 2, Side::Bid, 10, 101 );
    book.onEvent( on2 );
    OrderNew on3( 3, Side::Offer, 13, 102 );
    book.onEvent( on3 );

    BOOST_REQUIRE( book.getBestBidPrice() == 10 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 13 );

    OrderModify om2( 2, 11, 102 );
    BOOST_REQUIRE( book.onEvent( om2 ) == 0 );
    BOOST_REQUIRE( book.getBestBidPrice() == 11 );

    OrderCancel oc2( 2 );
    book.onEvent( oc2 );
    BOOST_REQUIRE( book.getBestBidPrice() == 10 );

    OrderModify om3( 3, 14, 102 );
    BOOST_REQUIRE( book.onEvent( om3 ) == 0 );
    BOOST_REQUIRE( book.getBestOfferPrice() == 14 );

    OrderModify om3a( 3, 14, -102 );
    BOOST_REQUIRE( book.onEvent( om3a ) > 0 );

    OrderModify om3b( 3, -14, 102 );
    BOOST_REQUIRE( book.onEvent( om3b ) > 0 );
}

// Get time stamp in nanoseconds.
uint64_t nanos()
{
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    return ns; 
}

BOOST_AUTO_TEST_CASE( Speed )
{
    using namespace std;

    Book book("TestInst");

    double count = 0;
    unsigned int n = 500000;

    
    for( unsigned int i=0;i<n;i++)
    {
        OrderNew on1( i, Side::Bid, rand()%100+1, 100 );

        auto start = nanos();
        book.onEvent( on1 );
        auto end = nanos();
        double microseconds =  ( end - start ) / 1000.0;
        count += microseconds;
    }

    cout << "Avg new: " << ( count / n ) << "us\n";

    std::list< Order* > orders = book.getOrdersInRange( Side::Bid, 0, 100 );
    BOOST_REQUIRE( orders.size() == n );

    count = 0;
    for( Order* o : orders )
    {
        OrderModify om( o->getOrderId(), o->getPrice()+1.0, o->getQty()+1 );

        auto start = nanos();
        book.onEvent( om );
        auto end = nanos();
        double microseconds =  ( end - start ) / 1000.0;
        count += microseconds;
    }
    cout << "Avg modify: " << ( count / n ) << "us\n";
    orders = book.getOrdersInRange( Side::Bid, 0, 101 );
    BOOST_REQUIRE( orders.size() == n );
    BOOST_CHECK( book.getBestBidPrice() == 101 );

    count = 0;
    for( Order* o : orders )
    {
        OrderCancel oc( o->getOrderId() );

        auto start = nanos();
        book.onEvent( oc );
        auto end = nanos();
        double microseconds =  ( end - start ) / 1000.0;
        count += microseconds;
    }
    cout << "Avg cancel: " << ( count / n ) << "us\n";
    orders = book.getOrdersInRange( Side::Bid, 0, 100 );
    BOOST_REQUIRE( orders.size() == 0 );


} 

BOOST_AUTO_TEST_CASE( FindRange )
{
    using namespace std;


    auto tc = []( Side s )
    {
        Book book("TestInst");
        for( int i=0,j=0;i<20;i++)
        {
            OrderNew on1( j++, s, i, 100 );
            BOOST_REQUIRE( book.onEvent( on1 ) == 0 );
            OrderNew on2( j++, s, i, 101 );
            BOOST_REQUIRE( book.onEvent( on2 ) == 0 );
        }

        std::list< Order* > orders = book.getOrdersInRange( s, 11, 12 );
        for( auto o : orders )
            BOOST_REQUIRE( o->getPrice() >= 11 && o->getPrice() <= 12 );
            
        for( auto o : orders )
        {
            OrderCancel oc( o->getOrderId() );
            BOOST_REQUIRE( book.onEvent( oc ) == 0 );
        }

        orders = book.getOrdersInRange( s, 11, 12 );
        BOOST_REQUIRE( orders.size() == 0 );

        orders = book.getOrdersInRange( s, -10, 13 );
        for( auto o : orders )
            BOOST_REQUIRE( o->getPrice() < 11 || o->getPrice() == 13 );
        BOOST_REQUIRE(orders.size() == 24);
    };

    tc( Side::Bid );
    tc( Side::Offer );

}


BOOST_AUTO_TEST_SUITE_END()


