#include "evt/evt_pouch.h"
#include "evt/evt_cmd.h"
#include "mario/mario_pouch.h"
#include "mario/mario_party.h"

extern void partyLeft(s32 partnerId);
extern BOOL pouchRemoveItemIndex(s32 id, s32 index);

USER_FUNC(evt_pouch_get_coin) {
    s32* args = event->args;
    s32 val = pouchGetCoin();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_add_coin) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchAddCoin(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_set_coin) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchSetCoin(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_supercoin) {
    s32* args = event->args;
    s32 val = pouchGetSuperCoin();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_set_supercoin) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchSetSuperCoin(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_starpiece) {
    s32* args = event->args;
    s32 val = pouchGetStarPiece();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_add_starpiece) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchAddStarPiece(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_starstone) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchGetStarStone(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_add_item) {
    s32* args = event->args;
    s32 val = evtGetValue(event, args[0]);
    if (pouchGetItem(val) == TRUE) {
        evtSetValue(event, args[1], 0);
    } else {
        evtSetValue(event, args[1], -1);
    }
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_remove_item) {
    s32* args = event->args;
    s32 val = evtGetValue(event, args[0]);
    if (pouchRemoveItem(val) == TRUE) {
        pouchReviseMarioParam();
        pouchRevisePartyParam();
        evtSetValue(event, args[1], 0);
    } else {
        evtSetValue(event, args[1], -1);
    }
    return EVT_RETURN_DONE;
}

USER_FUNC(N_evt_pouch_remove_item_index) {
    s32* args = event->args;
    s32 id = evtGetValue(event, args[0]);
    s32 idx = evtGetValue(event, args[1]);
    if (pouchRemoveItemIndex(id, idx) == TRUE) {
        pouchReviseMarioParam();
        pouchRevisePartyParam();
        evtSetValue(event, args[2], 0);
    } else {
        evtSetValue(event, args[2], -1);
    }
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_add_keepitem) {
    s32* args = event->args;
    s32 val = evtGetValue(event, args[0]);
    if (pouchAddKeepItem(val) == TRUE) {
        evtSetValue(event, args[1], 0);
    } else {
        evtSetValue(event, args[1], -1);
    }
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_remove_keepitem) {
    s32* args = event->args;
    s32 id = evtGetValue(event, args[0]);
    s32 idx = evtGetValue(event, args[1]);
    if (pouchRemoveKeepItem(id, idx) == TRUE) {
        evtSetValue(event, args[2], 0);
    } else {
        evtSetValue(event, args[2], -1);
    }
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_check_item) {
    s32* args = event->args;
    s32 val = evtGetValue(event, args[0]);
    s32 chk = pouchCheckItem(val);
    evtSetValue(event, args[1], chk);
    return EVT_RETURN_DONE;
}

USER_FUNC(L_evt_pouch_get_hp) {
    s32* args = event->args;
    s32 val = pouchGetHP();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(L_evt_pouch_set_hp) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchSetHP(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_max_ap) {
    s32* args = event->args;
    s32 val = pouchGetMaxAP();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_set_ap) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchSetAP(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_party_join) {
    s32 val = evtGetValue(event, event->args[0]);
    partyJoin(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_party_left) {
    s32 val = evtGetValue(event, event->args[0]);
    partyLeft(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_mario_recovery) {
    pouchSetHP(pouchGetMaxHP());
    pouchSetFP(pouchGetMaxFP());
    pouchSetAP(pouchGetMaxAP());
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_all_party_recovery) {
    PouchData* pouch = pouchGetPtr();
    pouch->partyData[0].currentHP = pouch->partyData[0].maximumHP;
    pouch->partyData[1].currentHP = pouch->partyData[1].maximumHP;
    pouch->partyData[2].currentHP = pouch->partyData[2].maximumHP;
    pouch->partyData[3].currentHP = pouch->partyData[3].maximumHP;
    pouch->partyData[4].currentHP = pouch->partyData[4].maximumHP;
    pouch->partyData[5].currentHP = pouch->partyData[5].maximumHP;
    pouch->partyData[6].currentHP = pouch->partyData[6].maximumHP;
    pouch->partyData[7].currentHP = pouch->partyData[7].maximumHP;
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_haveitemcnt) {
    s32* args = event->args;
    s32 val = pouchGetHaveItemCnt();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_havebadgecnt) {
    s32* args = event->args;
    s32 val = pouchGetHaveBadgeCnt();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_equipbadgecnt) {
    s32* args = event->args;
    s32 val = pouchGetEquipBadgeCnt();
    evtSetValue(event, args[0], val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_majinai_set) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchMajinaiInit(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_receive_mail) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchReceiveMail(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_open_mail) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchOpenMail(val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_set_yoshiname) {
    s32 val = evtGetValue(event, event->args[0]);
    pouchSetYoshiName((char*)val);
    return EVT_RETURN_DONE;
}

USER_FUNC(evt_pouch_get_yoshiname) {
    s32* args = event->args;
    char* name = pouchGetYoshiName();
    evtSetValue(event, args[0], (s32)name);
    return EVT_RETURN_DONE;
}
