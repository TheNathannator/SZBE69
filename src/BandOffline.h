#ifndef BANDOFFLINE_H
#define BANDOFFLINE_H

#include <map>
#include "system/utl/Str.h"
#include "system/obj/Data.h"

class BandOffline {
    static void Init();

    static void Poll();
    static DataNode MakeCharClipBudget(DataArray *da);
};

class CharStatKeeper {
public:
    std::map<int, float> mIntensitites;
    std::map<String, float> mGroups;

    void AddEq(const CharStatKeeper& other);
    void MaxEq(const CharStatKeeper& other);
    void ScaleEq(float factor);

    void OnPoll(int, String, float);
};

class SortGroupPolls {
    bool operator()(std::pair<String, float>& lh, std::pair<String, float>& rh) {
        return lh.second < rh.second;
    }
};

#endif
