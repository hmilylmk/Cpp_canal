#include "EntryProtocol.pb.h"
#include "protobuf.h"
#include <absl/log/log_entry.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <unistd.h>
#include <vector>

void printCol(const ::google::protobuf::RepeatedPtrField<com::alibaba::otter::canal::protocol::Column> & columns) {
    for (auto column : columns) {
        std::cout << column.name() << " : " << column.value() << " update=" << column.updated() << std::endl;
    }
}

void printEntry(const std::vector<::com::alibaba::otter::canal::protocol::Entry> &entrys) {
    for (auto entry : entrys) {
        if (entry.entrytype() == ::com::alibaba::otter::canal::protocol::EntryType::TRANSACTIONBEGIN || entry.entrytype() == ::com::alibaba::otter::canal::protocol::EntryType::TRANSACTIONEND) continue;
        com::alibaba::otter::canal::protocol::RowChange rowchange;
        rowchange.ParseFromString(entry.storevalue());
        com::alibaba::otter::canal::protocol::EventType eventType = rowchange.eventtype();
        std::cout << "binglog[" << entry.header().logfilename() << ":" << entry.header().logfileoffset() << "], name[" << entry.header().schemaname() << ":" << entry.header().tablename() << "], eventType: " << eventType << std::endl;

        const ::google::protobuf::RepeatedPtrField<::com::alibaba::otter::canal::protocol::RowData> &rowdatalist = rowchange.rowdatas();
        for (auto rowdata : rowdatalist) {
            if (eventType == com::alibaba::otter::canal::protocol::EventType::DELETE) {
                printCol(rowdata.beforecolumns());
            }
            else if (eventType == com::alibaba::otter::canal::protocol::EventType::INSERT) {
                printCol(rowdata.aftercolumns());
            }
            else {
                std::cout << "before: " << std::endl;
                printCol(rowdata.beforecolumns());
                std::cout << "after: " << std::endl;
                printCol(rowdata.aftercolumns());
            }
        }
    }
}

int main(void)
{
    Client c1("127.0.0.1", 11111);
    c1.connect();
    c1.checkValid("canal", "canal");
    c1.subscribe("1001", "example", "test.user");
    int emptyCount = 0;
    int totalEmptyCount = 120;
    while (emptyCount < totalEmptyCount) {
        int batchId = 0;
        std::vector<::com::alibaba::otter::canal::protocol::Entry> messages = c1.getWithoutAck(100, -1, -1, &batchId);
        if (batchId == -1 || messages.size() == 0) {
            emptyCount++;
            std::cout << "empty count ..." << std::endl;
            sleep(1);
        }
        else {
            emptyCount = 0;
            printEntry(messages);
        }
    }
    return 0;
}
