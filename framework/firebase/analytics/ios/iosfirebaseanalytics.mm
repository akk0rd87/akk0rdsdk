#include "iosfirebaseanalytics.h"
#import <FirebaseCore/FirebaseCore.h>

void Firebase::Analytics::iOS::Init() {
    [FIRApp configure];
}
