#include <iostream>

#include <docopt.h>
#include "golpe.h"

#include "DBScan.h"
#include "events.h"


static const char USAGE[] =
R"(
    Usage:
      scan [--pause=<pause>] [--metrics] <filter>
)";


void cmd_scan(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    uint64_t pause = 0;
    if (args["--pause"]) pause = args["--pause"].asLong();

    bool metrics = args["--metrics"].asBool();

    std::string filterStr = args["<filter>"].asString();


    auto filterGroup = NostrFilterGroup::unwrapped(tao::json::from_string(filterStr), MAX_U64);
    Subscription sub(1, "junkSub", filterGroup);
    DBScanQuery query(sub);


    Decompressor decomp;

    auto txn = env.txn_ro();

    while (1) {
        bool complete = query.process(txn, pause ? pause : MAX_U64, metrics, [&](const auto &sub, uint64_t levId){
            std::cout << getEventJson(txn, decomp, levId) << "\n";
        });

        if (complete) break;
    }
}
