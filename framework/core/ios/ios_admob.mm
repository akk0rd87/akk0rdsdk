//#include "SDL.h"
#include "ios_admob.h"
#import <GoogleMobileAds/GoogleMobileAds.h>

bool AdMobiOS::Init(const char* PublisherID, int Formats)
{
    NSString *PubID = [[NSString alloc] initWithUTF8String:PublisherID];
    [GADMobileAds configureWithApplicationID:PubID];
    [PubID release];    
    return true;
}
