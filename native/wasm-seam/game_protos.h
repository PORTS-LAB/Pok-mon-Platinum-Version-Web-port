#pragma once
/*
 * Prototypes missing upstream. mwcc tolerates implicit declarations;
 * clang errors on them. Each entry notes the defining TU.
 * The decomp itself is never modified; declare the truth here.
 *
 * TRAP: upstream include/link_contest_records.h reuses the guard
 * POKEPLATINUM_CONTEST_H — same as include/contest.h. So including both
 * makes one an empty no-op. Fix: re-implement link_contest_records.h's
 * contents here under a distinct guard and shadow the upstream file with
 * shim/link_contest_records.h that includes this one.
 */
#include "player_avatar.h"
#include "chatot_cry.h"
#include "graphics.h"
#include "wifi_history_save_data.h"
#include "contest.h"
#include "applications/capsule_menu/defs.h"
#include "applications/pokedex/pokedex_sort.h"
#include "battle/battle_controller.h"

/* Prototypes upstream declares only implicitly (mwcc tolerates implicit
 * decls; clang errors). */
int PlayerAvatar_GetMoveState(const PlayerAvatar *playerAvatar);
void UndergroundMan_RemovePrinters(void);
void BattleController_EmitSelectedMove(BattleSystem *battleSys, int battler, int command);
void BattleController_EmitSelectedCommand(BattleSystem *battleSys, int battler, int command);
void BattleController_EmitSelectedTarget(BattleSystem *battleSys, int battler, int command);
void CapsuleGraphics_Dummy(CapsuleAppManager *appMan);

#ifndef POKEPLATINUM_WASM_SEAM_LINK_CONTEST_RECORDS_H
#define POKEPLATINUM_WASM_SEAM_LINK_CONTEST_RECORDS_H

typedef struct LinkContestRecords {
    u16 contestPlacement[CONTEST_TYPE_MAX][CONTEST_NUM_PARTICIPANTS];
} LinkContestRecords;

int LinkContestRecords_SaveSize(void);
void LinkContestRecords_Init(LinkContestRecords *linkContestRecords);
void LinkContestRecords_IncrementSavaData(SaveData *saveData, enum PokemonContestType contestType, int placement);
u16 LinkContestRecords_GetSavaData(SaveData *saveData, enum PokemonContestType contestType, int placement);

#endif

void CapsuleGraphics_LoadBgLayer2Tilemap(CapsuleAppManager *appMan, NARC *narc);
int SealManger_CapsuleMenuField(CapsuleAppManager *appMan, int index);
int PokedexSort_NumDisplayed(const PokedexSortData *param0);
