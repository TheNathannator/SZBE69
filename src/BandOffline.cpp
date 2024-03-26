#include "BandOffline.h"

#include <algorithm>

#include "system/obj/DataFunc.h"
#include "system/obj/Dir.h"
#include "system/obj/Msg.h"
#include "system/os/Debug.h"
#include "system/os/System.h"
#include "system/os/PlatformMgr.h"
#include "system/utl/Option.h"

void BandOffline::Init() {
    DataRegisterFunc("make_charbudget", MakeCharClipBudget);
}

DataNode BandOffline::MakeCharClipBudget(DataArray *da) {
    TextFileStream log(OptionStr("budget_log", "charclipbudget_log.csv"), false);

    TheContentMgr.0x68();
    while (!TheContentMgr.RefreshDone()) {
        Poll();
    }

    Hmx::Object* cb = Hmx::Object::New<Hmx::Object>();
    cb->SetName("cb", ObjectDir::Main());
    cb->SetType("charbudget");

    DataArray* songs = SystemConfig("charbudget_songs");

    for (int i = 0; i < 3; i++) {
        CharStatKeeper totals[4];
        CharStatKeeper maxes[4];
        CharStatKeeper normalMaxes[4];

        std::vector<stlpmtx_std::pair<Symbol,float> > tempoSongs;
        float totalDuration = 0;

        Symbol slow("slow");
        for (int j = 1; j < songs->Size(); j++) {
            Symbol song = songs->Sym(j);
            Message get_song_tempo("get_song_tempo", song);
            if (cb->HandleType(get_song_tempo).Sym(NULL) != slow)
                continue;

            MILO_LOG("Loading %s", song);
            CharStatKeeper stats[4];
            float duration = 0;
            while (TheUI.InTransition()) {
                BandOffline::Poll();
            }

            Message run_session("run_session", song);
            cb->HandleType(run_session);

            BandCharacter* dudes[4] = { NULL };
            int isDone;
            do {
                BandOffline::Poll();

                static Message is_active("is_active");
                if (cb->HandleType(is_active).Int(NULL)) {
                    if (dudes[0] == NULL) {
                        static Message chars_dir("chars_dir");

                        int index = 0;
                        for (ObjDirIter<BandCharacter*> dude : TheBandDirector.0x04.0x10(chars_dir, true).Obj<ObjectDir>(NULL)) {
                            dudes[index] = dude;
                            index++;
                        }
                    }

                    duration += TheTaskMgr.DeltaSeconds();
                    for (int i2 = 0; i2 < ARRAY_LENGTH(dudes); i2++) {
                        BandCharacter* dude = dudes[i2];
                        stats[GetStatKeeperIndex(dude->Name())]->OnPoll(
                            dude->GetPlayFlags(),
                            dude->GetGroupName(),
                            TheTaskMgr.DeltaSeconds()
                        );
                    }
                }

                static Message is_done("is_done");
                isDone = cb->HandleType(is_done).Int(NULL);
            } while (!isDone);

            tempoSongs.push_back(std::make_pair(song, duration));
            for (int i2 = 0; i2 < ARRAY_LENGTH(stats); i2++) {
                CharStatKeeper& stat = stats[i2];
                totals[i2].AddEq(stat);
                maxes[i2].MaxEq(stat);
                stat.ScaleEq(1.0 / duration);
                normalMaxes[i2].MaxEq(stat);
            }

            totalDuration += duration;
        }

        int numSongs = tempoSongs.size();
        float invSongs = numSongs == 0 ? 0.0 : 1.0 / numSongs;

        log << "\nTempo,Section,Song,Duration (s)\n";
        for (int i2 = 0; i2 < numSongs; i2++) {
            log << slow << ","
                << "songs" << ","
                << tempoSongs[i2].first << ","
                << tempoSongs[i2].second << "\n";
        }

        log << "\nTempo,Section,Category,Animation,Average (s),Max (s),Normalized Max (s)\n";
        static char* insts[4] = { "george", "john", "paul", "ringo" };
        for (int i2 = 0; i2 < ARRAY_LENGTH(insts); i2++) {
            totals[i2].ScaleEq(invSongs);

            std::map<int, float> intensities; // TODO: This map never seems to be initialized, find where it comes from
            for (std::map<int, float>::iterator it = intensities.begin(); it != intensities.end(); it++) {
                int intensity = it->first;
                float norMax = normalMaxes[i2].mIntensitites[intensity];
                float max = maxes[i2].mIntensitites[intensity];

                log << slow << ","
                    << insts[i2] << ","
                    << "intensities" << ","
                    << BandIntensityString(it->first) << ","
                    << it->second << ","
                    << max << ","
                    << totalDuration * invSongs * norMax << "\n";
            }

            std::vector<std::pair<String, float> > vec;
            std::map<String, float> groups; // TODO: This map never seems to be initialized, find where it comes from
            for (std::map<String, float>::iterator it = groups.begin(); it != groups.end(); it++) {
                String groupName = it->first;
                Message is_valid_group("is_valid_group", groupName.c_str());
                if (cb->HandleType(is_valid_group).Int(NULL) != 0) {
                    std::pair<String, float> pair = *it;
                    vec.push_back(pair);
                }
            }

            std::sort(vec.begin(), vec.end(), SortGroupPolls());
            for (int i2 = 0; i2 < vec.size(); i2++) {
                String group = vec[i2].first;
                float norMax = normalMaxes[i2].mGroups[group];
                float max = maxes[i2].mGroups[group];

                log << slow << ","
                    << insts[i2] << ","
                    << "groups" << ","
                    << group.c_str() << ","
                    << vec[i2].second << ","
                    << max << ","
                    << totalDuration * invSongs * norMax << "\n";
            }
        }
    }

    log << "\nDone\n";
    delete cb;
    TheDebug.Exit(0, true);
    return DataNode(1);
}

void BandOffline::Poll() {
    SystemPoll(false);
    TheSynth.0x88();
    ThePlatformMgr.Poll();
    TheNet.Poll();
    TheRockCentral.Poll();
    TheUI.0x04.0x18();
    TheTaskMgr.Poll();
    TheRnd.0x98();
    TheUI.0x04.0x1C();
    TheRnd.0x9C();
}
