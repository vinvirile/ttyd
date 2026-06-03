/* "evt_case.c" - Event case commands */

#include "evt/evt_cmd.h"
#include "manager/evtmgr.h"
#include "manager/evtmgr_cmd.h"
#include "driver/casedrv.h"
#include <string.h>

s32 evt_run_case_evt(EventEntry* event) {
    s32* args = event->args;
    s32 v0 = evtGetValue(event, args[0]);
    s32 v1 = evtGetValue(event, args[1]);
    s32 v2 = evtGetValue(event, args[2]);
    s32 v3 = evtGetNumber(event, args[3]);
    s32 v4 = evtGetValue(event, args[4]);
    CaseSetup setup;
    CaseSetup* sp = &setup;
    s32 condId;

    condId = v0;
    if (v1 != 0) {
        condId = v0 | 0x8000;
    }
    sp->activeConditionId = (u16)condId;
    sp->hitObjName = (const char*)v2;
    sp->swFlag = v3;
    sp->activeFunc = NULL;
    sp->evtCode = (void*)v4;
    sp->priority = 0;
    memcpy(sp->lwData, event->lwData, 0x40);
    v4 = caseEntry(sp);
    if (args[5] != 0) {
        evtSetValue(event, args[5], v4);
    }
    return EVT_RETURN_DONE;
}

s32 evt_run_case_evt_bero(EventEntry* event) {
    s32* args = event->args;
    s32 v0 = evtGetValue(event, args[0]);
    s32 v1 = evtGetValue(event, args[1]);
    s32 v2 = evtGetValue(event, args[2]);
    s32 v3 = evtGetNumber(event, args[3]);
    s32 v4 = evtGetValue(event, args[4]);
    CaseSetup setup;
    CaseSetup* sp = &setup;
    s32 condId;

    condId = v0;
    if (v1 != 0) {
        condId = v0 | 0x8000;
    }
    sp->activeConditionId = (u16)condId;
    sp->hitObjName = (const char*)v2;
    sp->swFlag = v3;
    sp->activeFunc = NULL;
    sp->evtCode = (void*)v4;
    sp->priority = 0x14;
    memcpy(sp->lwData, event->lwData, 0x40);
    v4 = caseEntry(sp);
    if (args[5] != 0) {
        evtSetValue(event, args[5], v4);
    }
    return EVT_RETURN_DONE;
}

s32 evtRunCaseEntry(s32 activeConditionId, s32 inBattle, const char* hitObjName,
                    s32 swFlag, void* evtCode, void* setupData) {
    CaseSetup setup;
    CaseSetup* sp = &setup;

    if (inBattle != 0) {
        activeConditionId |= 0x8000;
    }
    sp->activeConditionId = (u16)activeConditionId;
    sp->hitObjName = hitObjName;
    sp->swFlag = swFlag;
    sp->activeFunc = NULL;
    sp->evtCode = evtCode;
    sp->priority = 0;
    if (setupData != NULL) {
        memcpy(sp->lwData, setupData, 0x40);
    } else {
        memset(sp->lwData, 0, 0x40);
    }
    return caseEntry(sp);
}

s32 evt_exit_case_evt(EventEntry* event) {
    caseDelete(event->caseId);
    return EVT_RETURN_DONE;
}

s32 evt_del_case_evt(EventEntry* event) {
    s32* args = event->args;
    s32 v0 = evtGetValue(event, args[0]);
    s32 v1 = evtGetValue(event, args[1]);

    if (v0 == 0) {
        CaseEntry* entry = caseIdToPtr(v1);
        if (evtCheckID(entry->eventId)) {
            evtDeleteID(entry->eventId);
        }
    }
    caseDelete(v1);
    return EVT_RETURN_DONE;
}

s32 evt_set_case_wrk(EventEntry* event) {
    s32* args = event->args;
    s32 caseId = evtGetValue(event, args[0]);
    s32 caseIndex = evtGetValue(event, args[1]);
    s32 value = evtGetValue(event, args[2]);
    CaseEntry* entry = caseIdToPtr(caseId);

    entry->lwData[caseIndex] = value;
    return EVT_RETURN_DONE;
}
