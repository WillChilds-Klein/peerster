GroupThink
===========================
Will Childs-Klein & Miles Richardson

Table of Contents
-----------------
[1. Overview](#Overview)
[2. Incentive Structures](#Incentive-Structures)
[3. Components](#Components)
[4. Technologies](#Technologies)
[5. Potential Vulnerabilities](#Potential-Vulnerabilities)
[6. Milestones](#Milestones)
[7. Further Work](#Further-Work)
[8. Miscellaneous](#Miscellaneous)

Overview
--------
GroupThink is a a reputation-based subjective recommendation network which
recommends the most profitable alt-coin to mine for an arbitrary point in time.
The GroupThink Miner handles the logistics of changing mining operations
automatically. This automated mine-switching (on the basis of coin
profitability) is directly inspired by [CryptoSwitcher][1]. GroupThink improves
on this concept by expanding the way in which profitability is calculated.
Individual nodes are able to leverage the network's opinions to optimize mining
profitability by mining different coins as the network recommends. All nodes on
the network are responsible for generating expected profitability scores on
demand, taking a coin name/id and arbitrary point in time as parameters. Nodes
request coins' expected profitabilities from other nodes on the network in
discretized _voting rounds_, which occur periodically. GroupThink is subjective
in that it allows nodes to define their own criteria for profitability, as well
as how they calculate expected profitability. The resulting differences in
expected profitability are the driving force of GroupThink's strategy: querying
a crowd with (hopefully) diverse opinions and amplifying the most historically
useful ones.

Incentive Structures
-------------------
Nodes are incentivized to act in good faith (report accurate estimates to the
best of their ability) because doing so over time will cause successful nodes to
accrue "followers", giving the successful nodes greater sway in coin
recommendation on the network. Symmetrically, nodes who perform poorly over time
(either by malice or ineptitude) will be regarded as less reputable, and 
therefore have respectively less sway over other nodes. 

Coins with high degrees of mining activity tend to attract the attention of
market prospectors. Such attention often results in an influx of capital into
the coin's market capitalization, compounding profitability. But, as more nodes
mine a given currency, that currecy's mining difficulty will increase, reducing
profitability. GroupThink collectively senses this, and the coin's ranking
drops, yielding new coin recommendations as other coins climb the rankings to
fill the void. 

Components
----------
### 1. Subjective Utility Function ###
  * default is very similar to the way Credence calculates reputation
  * default considers entire voting history of a peer (specifically, how
    accurately the peer's voting history predicted empirical profitability) and
    returns a subjective reputation score of that peer. this process is repeated
    for as many peers as the node wishes to consider the opinions of.
  * function and output are private, as exposing this function would expose
    nodes with non- normative utiltiy criteria to risk of being gamed by
    malicious peers.
  * it is _recommended_ that nodes use similar criteria to the default utility
    function, but it is not required.
  * a node can potentially gain important and novel insight by selecting a
    utility basis which differs from the default

### 2. Expected Profitability Function ###
  * EP<sub>i</sub>(C<sub>k</sub> , t<sub>x</sub>)
    * EP<sub>i</sub>: node i's expected profitability function
    * C<sub>k</sub>: an arbitrary coin
    * t<sub>x</sub>: an arbitrary time
  * this is the component which primarily distinguishes nodes in GroupThink and
    drives its collaboratively competitive ecosystem.
  * node is responsible for sourcing its own information to use in this function
  * nodes can calculate this by any means they see fit, and are encouraged to
    undertake novel and innovative approaches
  * naturally, some nodes will be more clever than others, creating subjective
    distinctions in reputations amongst nodes.
  * should be singularly defined by node, but GroupThink will ship with a
    default, naive function
    * basic weighting of coin difficulty and price, each extrapolated via linear
      regression

### 3. Vote Aggregator ###
  * The Vote Aggregator is responsible for gather all nodes (or a random
    sampling of nodes. this is at the discretion of the individual nodes)
  * It is also responsible for batching vote queries
  * On startup, the vote aggregator is very busy. It has to query all nodes on
    the network for their voting histories on all coins the node might wish to
    consider.

### 4. Database ###
  * will almost certainly us an ORM of some sort.
  * stores historical voting data of peers
  * simulate verifiable voting histories by having nodes store their own voting
    histories, making those contents readily available to new nodes to the
    network.
  * essentially, this is just a proxy for the blockchain [history](#Verifiable
    History)

### 5. Miner Suite ###
  * suite of scripts, each calibrated to mine a specific coin.
  * switcher object takes coin name/id as parameter and hot-swaps mining scripts
    accordingly

### 6. Messaging System ###
  * message types
    * request: [{from},{coin},{time}]
    * reply: [{from},{coin},{time},{ep}]
    * feilds
      * __from__: node sending message
      * __coin__: name/id of coin in question, parameter for requestee's expected
        profitability
      * __time__: time parameter for requestee's expected profitability
      * __ep__: the requestee's expected profitability for the specified
        ({coin},{time}) pair.
  * rudimentary JSON messages sent via HTTP

Technologies
------------
  * [Python Flask][2]
  * [SQL-Alchemy][3]
  * [SQLite][7]
  * [JSON][4]
  * [numpy][5]
  * [mininet][6]
  * [CryptoSwicher][1]

Potential Vulnerabilities
-------------------------
### Noisy Sybil Attack ###
  * Eve spins up a large number of independent GroupThink instances, each having
    a different or randomized expected profitability function.
  * The large number nodes flood GroupThink with random noise that dilutes the
    signal of honest nodes.
  * Honest nodes should then gain reputation relative to the nodes spawned by
    Eve, as Eve's nodes will prove to be useless over time and therefore lose
    reputability.

### Sybil-Brutus Attack ###
  * Brutus spins up large number puppet GroupThink instances and controls their
    expected profitability votes from a single location.
  * Brutus's profitability expectations pretty smart, and his nodes generate 
    very accurate estimates which appeal to many nodes' respctive dispositions 
    (utility criteria).  
  * Brutus's nodes operate in (seemingly) good faith for a long period of time,
    gaining a strong reputation among like-minded (similar utility criteria) 
    nodes in GroupThink.
  * Brutus releases her new alt-coin, EtTuCoin, and inflates her nodes' expected
    profitability for EtTuCoin.
  * Though many nodes on the network will then be operating on bad-faith
    information, mining coins of sub-optimal profitability, their internal 
    utility expected profitabilities and reduce the weight for his nodes 
    accordingly.

### Cold Start Attack ###
  * New nodes build voting histories of their peers, and therefore their
    subjective reputations of those peers, by requesting votes directly from the
    peer in question.
  * It would be to the advantage of malicious nodes to recognize requests for
    past votes and generate spurious responses based on empirical data.
  * This would game the new node into regarding the malicious node highly under
    false pretenses, opening the door for the malicious node to manipulate
    behavior of new nodes at will.

### Reputation Markets ###
  * Because this system is so meritocratic, there exists the potential for a few
    nodes to by hyper-successful and begin to sway many recommendations.
  * This can create the opportunity for a _reputation market_ wherein a
    malicious party offers widely reputable nodes payment in exchange for their
    identities and therefore their reputation.
  * By doing this to many highly reputable nodes, it is feasible that an
    attacker could single-handedly control the collective opinion of the
    GroupThink network

Milestones
----------
### Part 1 ###
  [ ] Messaging System
  [ ] Database

### Part 2 ###
  [ ] Expected Profitability Function (multiple examples)
  [ ] Subjective Utility Function

### Part 3 ###
  [ ] Vote Aggregator 
  [ ] Mining Suite

Further Work
------------
### Verifiable History ###
  * use a shared blockchain as a means of verifying the recommendation history 
    of individual nodes, so that a new node joining the network can accurately 
    gauge a subjective reputation of its peers.
  * this eliminates the possibility of a node falsifying its voting history.

### Rapid Liquidator ###
  * Because profitability is often correlated with price, it would be advantageous for nodes to automatically liquidate coins they mine to BTC or USD 
  * node defines rules of when to liquidate
  * logical time would be immediately liquidating entire chunk of mined coins the second GroupThink moves away from a coin
  * could potentially set up the liquidator to flow mined capital through alt-coins as they are recommended/mined. in this case, Rapid Liquidator would be transferring currency through exchanges per the mining recommendations of GroupThink.

Miscellaneous
-------------
### Opinion ###

### Coin-Hedge Profile ###

### Volatility Profile ###

### Passivity & Monitoring ###


[1]: https://github.com/area/CryptoSwitcher
[2]: http://flask.pocoo.org/
[3]: http://www.sqlalchemy.org/
[4]: http://www.json.org/
[5]: http://www.numpy.org/
[6]: http://mininet.org/
[7]: http://www.sqlite.org/