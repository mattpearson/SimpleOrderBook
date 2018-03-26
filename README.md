# SimpleOrderBook
Order Book that maps prices to orders. Handles New, Modify, Delete operations. 
Could be base for Market-By-Order implementations.
Single threaded usage.
Uses std::map to map prices to boost::intrusive list of orders.
