/*!
 * @file main-win-music.cpp
 * @brief Windows版固有実装(BGM)
 */

#include "main-win/main-win-music.h"
#include "dungeon/quest.h"
#include "floor/floor-town.h"
#include "main-win/main-win-define.h"
#include "main-win/main-win-file-utils.h"
#include "main-win/main-win-mci.h"
#include "main-win/main-win-mmsystem.h"
#include "main-win/main-win-tokenizer.h"
#include "main/scene-table.h"
#include "main/sound-of-music.h"
#include "monster-race/monster-race.h"
#include "term/z-term.h"
#include "util/angband-files.h"
#include "world/world.h"

bool use_pause_music_inactive = false;
int current_music_type = TERM_XTRA_MUSIC_MUTE;
int current_music_id = 0;
// current filename being played
system_string current_music_path;

/*
 * Directory name
 */
concptr ANGBAND_DIR_XTRA_MUSIC;

/*
 * "music.cfg" data
 */
CfgData *music_cfg_data;

namespace main_win_music {

/*!
 * @brief action-valに対応する[Basic]セクションのキー名を取得する
 * @param index "term_xtra()"の第2引数action-valに対応する値
 * @param buf 使用しない
 * @return 対応するキー名を返す
 */
static concptr basic_key_at(int index, char *buf)
{
    (void)buf;

    if (index >= MUSIC_BASIC_MAX)
        return NULL;

    return angband_music_basic_name[index];
}

static inline DUNGEON_IDX get_dungeon_count()
{
    return current_world_ptr->max_d_idx;
}

/*!
 * @brief action-valに対応する[Dungeon]セクションのキー名を取得する
 * @param index "term_xtra()"の第2引数action-valに対応する値
 * @param buf バッファ
 * @return 対応するキー名を返す
 */
static concptr dungeon_key_at(int index, char *buf)
{
    if (index >= get_dungeon_count())
        return NULL;

    sprintf(buf, "dungeon%03d", index);
    return buf;
}

static inline QUEST_IDX get_quest_count()
{
    return max_q_idx;
}

/*!
 * @brief action-valに対応する[Quest]セクションのキー名を取得する
 * @param index "term_xtra()"の第2引数action-valに対応する値
 * @param buf バッファ
 * @return 対応するキー名を返す
 */
static concptr quest_key_at(int index, char *buf)
{
    if (index >= get_quest_count())
        return NULL;

    sprintf(buf, "quest%03d", index);
    return buf;
}

static inline TOWN_IDX get_town_count()
{
    return max_towns;
}

/*!
 * @brief action-valに対応する[Town]セクションのキー名を取得する
 * @param index "term_xtra()"の第2引数action-valに対応する値
 * @param buf バッファ
 * @return 対応するキー名を返す
 */
static concptr town_key_at(int index, char *buf)
{
    if (index >= get_town_count())
        return NULL;

    sprintf(buf, "town%03d", index);
    return buf;
}

static inline MONRACE_IDX get_monster_count()
{
    return max_r_idx;
}

/*!
 * @brief action-valに対応する[Monster]セクションのキー名を取得する
 * @param index "term_xtra()"の第2引数action-valに対応する値
 * @param buf バッファ
 * @return 対応するキー名を返す
 */
static concptr monster_key_at(int index, char *buf)
{
    if (index >= get_monster_count())
        return NULL;

    sprintf(buf, "monster%04d", index);
    return buf;
}

/*!
 * @brief BGMの設定を読み込む。
 * @details
 * "music_debug.cfg"ファイルを優先して読み込む。無ければ"music.cfg"ファイルを読み込む。
 * この処理は複数回実行されることを想定していない。複数回実行した場合には前回読み込まれた設定のメモリは解放されない。
 */
void load_music_prefs()
{
    CfgReader reader(ANGBAND_DIR_XTRA_MUSIC, { "music_debug.cfg", "music.cfg" });

    system_string device_type(mci_device_type.get_size());
    GetPrivateProfileStringA("Device", "type", "MPEGVideo", device_type.raw_ptr(), device_type.get_size(), reader.get_cfg_path());
    mci_device_type = to_utf16(device_type);

    // clang-format off
    music_cfg_data = reader.read_sections({
        { "Basic", TERM_XTRA_MUSIC_BASIC, basic_key_at },
        { "Dungeon", TERM_XTRA_MUSIC_DUNGEON, dungeon_key_at },
        { "Quest", TERM_XTRA_MUSIC_QUEST, quest_key_at },
        { "Town", TERM_XTRA_MUSIC_TOWN, town_key_at },
        { "Monster", TERM_XTRA_MUSIC_MONSTER, monster_key_at, &has_monster_music }
        });
    // clang-format on

    if (!has_monster_music) {
        int type = TERM_XTRA_MUSIC_BASIC;
        for (int val = MUSIC_BASIC_UNIQUE; val <= MUSIC_BASIC_HIGHER_LEVEL_MONSTER; val++) {
            if (music_cfg_data->has_key(type, val)) {
                has_monster_music = true;
                break;
            }
        }
    }
}

/*
 * Stop a music
 */
errr stop_music(void)
{
    mciSendCommandW(mci_open_parms.wDeviceID, MCI_STOP, MCI_WAIT, 0);
    mciSendCommandW(mci_open_parms.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);
    current_music_type = TERM_XTRA_MUSIC_MUTE;
    current_music_id = 0;
    current_music_path = system_string::empty();
    return 0;
}

/*
 * Play a music
 */
errr play_music(int type, int val)
{
    if (type == TERM_XTRA_MUSIC_MUTE)
        return stop_music();

    if (current_music_type == type && current_music_id == val)
        return 0; // now playing

    concptr filename = music_cfg_data->get_rand(type, val);
    if (!filename)
        return 1; // no setting

    system_string path;
    path_build(path.raw_ptr(), path.get_size(), ANGBAND_DIR_XTRA_MUSIC, filename);

    if (current_music_type != TERM_XTRA_MUSIC_MUTE)
        if (0 == strcmp(current_music_path.c_ptr(), path.c_ptr()))
            return 0; // now playing same file

    current_music_type = type;
    current_music_id = val;
    current_music_path = path;

    utf16_string tmp = to_utf16(path);
    mci_open_parms.lpstrDeviceType = mci_device_type.c_ptr();
    mci_open_parms.lpstrElementName = tmp.c_ptr();
    MCIERROR e;
    MCIERR_BAD_TIME_FORMAT;
    e = mciSendCommandW(mci_open_parms.wDeviceID, MCI_STOP, MCI_WAIT, 0);
    e = mciSendCommandW(mci_open_parms.wDeviceID, MCI_CLOSE, MCI_WAIT, 0);
    e = mciSendCommandW(mci_open_parms.wDeviceID, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_NOTIFY, (DWORD)&mci_open_parms);
    // Send MCI_PLAY in the notification event once MCI_OPEN is completed
    return 0;
}

/*
 * Pause a music
 */
void pause_music(void)
{
    mciSendCommandW(mci_open_parms.wDeviceID, MCI_PAUSE, MCI_WAIT, 0);
}

/*
 * Resume a music
 */
void resume_music(void)
{
    mciSendCommandW(mci_open_parms.wDeviceID, MCI_RESUME, MCI_WAIT, 0);
}

/*
 * Play a music matches a situation
 */
errr play_music_scene(int val)
{
    // リストの先頭から順に再生を試み、再生できたら抜ける
    auto &list = get_scene_type_list(val);
    const errr err_sucsess = 0;
    for (auto &item : list) {
        if (play_music(item.type, item.val) == err_sucsess) {
            break;
        }
    }

    return 0;
}

/*
 * Notify event
 */
void on_mci_notify(WPARAM wFlags, LONG lDevID)
{
    if (wFlags == MCI_NOTIFY_SUCCESSFUL) {
        // play a music (repeat)
        mciSendCommandW(lDevID, MCI_SEEK, MCI_SEEK_TO_START | MCI_WAIT, 0);
        mciSendCommandW(lDevID, MCI_PLAY, MCI_NOTIFY, (DWORD)&mci_play_parms);
    }
}

}
