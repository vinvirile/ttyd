#include "data/mail_data.h"

extern MailEntry mail_data[44];

MailEntry* mailGetPtr(s32 mailId) {
    MailEntry* entry = mail_data;
    s32 i;
    
    for (i = 0; i < 11; i++) {
        if (mailId == entry[0].mailId) return &entry[0];
        if (mailId == entry[1].mailId) return &entry[1];
        if (mailId == entry[2].mailId) return &entry[2];
        if (mailId == entry[3].mailId) return &entry[3];
        entry += 4;
    }
    return entry;
}
