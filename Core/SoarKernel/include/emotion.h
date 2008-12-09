#ifndef EMOTION_H
#define EMOTION_H

typedef struct agent_struct agent;
typedef struct wme_struct wme;


struct AppraisalStatus;
struct AppraisalFrame;
struct Mood;
struct Feeling;

struct emotion_data
{
	// emotion stuff
	AppraisalStatus* appraisalStatus;
	AppraisalFrame* currentEmotion;
	Mood* currentMood;
	Feeling* currentFeeling;
	wme* feeling_frame;
};

void cleanup_emotion_data(agent* thisAgent, emotion_data* ed);

void register_appraisal(emotion_data* ed, wme* appraisal);
void get_appraisals(Symbol* goal);
void update_mood(Symbol* goal);
void generate_feeling_frame(agent* thisAgent, Symbol* goal);
void emotion_reset_data( agent *thisAgent );

void emotion_update(agent* thisAgent, Symbol* goal);

#endif // EMOTION_H