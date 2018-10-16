#include "SDL.h"
#include "ios_wrapper.h"

#import <CoreFoundation/CFBundle.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

bool iOSWrapper::OpenURL(const char* url)
{
    // https://useyourloaf.com/blog/openurl-deprecated-in-ios10/
    @autoreleasepool
    {
        //NSLog(@"URL=%s", url);
        __block bool Result = false;
        
        UIApplication *app = [UIApplication sharedApplication];
        NSURL *nsurl = [NSURL URLWithString:@(url)];
    
        //if ([app canOpenURL:nsurl]) NSLog(@"Can open");
        //else NSLog(@"Cant't open");
        
    
        if([app respondsToSelector:@selector(openURL:options:completionHandler:)])
        {
            //NSLog(@"1");
            [app openURL:nsurl options:@{} completionHandler:^(BOOL success) {
                //NSLog(@"OpenURL %d", success);
                Result = true;
            }];
        }
        else
        {
            //NSLog(@"2");
            if ([app canOpenURL:nsurl])
            {
                [app openURL:nsurl];
                Result = true; 
            }
        }
        return Result;
    }
}

NSString *getDocumentsDirectory()
{
    NSArray *docdirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return docdirs[0];
}

void iOSWrapper::GetInternalWriteDir(std::string& Dir)
{
    NSArray *docdirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documents = docdirs[0];
    
    Dir = std::string([documents UTF8String]);
}

bool iOSWrapper::DirectoryExists(const char* Dir)
{
    NSString *path = [[NSString alloc] initWithUTF8String:Dir];
    
    BOOL isDir;       
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir];
    
    [path release];
    
    if(exists)
        if(isDir)
            return true;
    
    return false;
}

bool iOSWrapper::FileExists(const char* FileName)
{
    NSString *path = [[NSString alloc] initWithUTF8String:FileName];
    
    BOOL isDir;       
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir];
    
    [path release];
    
    if(exists)
    {
        if(isDir)          
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    
    return false;
}

bool iOSWrapper::DirCreate (const char* Dir)
{
    if (DirectoryExists(Dir))
        return true;
    
    bool res = false;
    NSError *error;
    NSString *dataPath = [[NSString alloc] initWithUTF8String:Dir];
    if (![[NSFileManager defaultManager] fileExistsAtPath:dataPath])
    
    if([[NSFileManager defaultManager] createDirectoryAtPath:dataPath withIntermediateDirectories:YES attributes:nil error:&error] == YES)
    {
        res = true;
    }

    [dataPath release];
    
    return res;
}

bool iOSWrapper::GetDirContent      (const char* Dir, DirContentElementArray& ArrayList)
{
    NSURL *DirURL = [NSURL URLWithString:@(Dir)];
    
    NSError *error;
    NSURL* URL;
    NSArray* dirContent = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:DirURL includingPropertiesForKeys:[NSArray arrayWithObjects:NSURLNameKey, NSURLIsDirectoryKey, nil] options:NSDirectoryEnumerationSkipsHiddenFiles error:&error];
    
    
    NSUInteger count;
    for(count = 0; count < [dirContent count]; ++count)
    {        
        std::unique_ptr<DirContentElement> dc (new DirContentElement());
        URL = [dirContent objectAtIndex:count];
        dc->isDir = URL.hasDirectoryPath;
        dc->Name  = std::string([URL.pathComponents[URL.pathComponents.count - 1] UTF8String]);        
        ArrayList.push_back(std::move(dc));
    }
    
    [dirContent release];
    
    return true;
}

bool iOSWrapper::DirRemoveRecursive (const char* Dir)
{
    if(DirectoryExists(Dir))
    {
        NSError *error;
        NSString *path = [[NSString alloc] initWithUTF8String:Dir];
        
        BOOL res = [[NSFileManager defaultManager] removeItemAtPath:path error:&error];
        
        [path release];
        
        if(res) return true;
        else    return false;
    }
    
    return true;
}

std::string iOSWrapper::GetLanguage()
{
    NSString * language = [[NSLocale preferredLanguages] firstObject];
    return std::string([language UTF8String]);
}

void iOSWrapper::MessageBoxShow (int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3)
{
	SDL_MessageBoxButtonData buttons[3];
	int buttonCnt = 1;

	buttons[0].buttonid = 0;
	buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	buttons[0].text = Button1;

	// Если есть вторая кнопка
	if (Button2 != nullptr && Button2[0] != '\0')
	{
		++buttonCnt;
		buttons[1].buttonid = 1;
		buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		buttons[1].text = Button2;
	}

	// Если есть третья кнопка
	if (Button3 != nullptr && Button3[0] != '\0')
	{
		++buttonCnt;
		buttons[2].buttonid = 2;
		buttons[2].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		buttons[2].text = Button3;

		buttons[1].flags = 0;
	}

    const SDL_MessageBoxColorScheme colorScheme = {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 88, 135, 63 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            { 250, 250, 250 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 255, 255, 0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            { 0, 0, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 255, 0, 255 }
        }
    };

    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        //NULL, /* .window */
        //CurrentContext.CurrentWindow,
		BWrapper::GetActiveWindow(),
        //NULL,
		Title, /* .title */
        Message, /* .message */
		buttonCnt, /* .numbuttons */
        buttons, /* .buttons */
        &colorScheme /* .colorScheme */
    };

    int buttonid;

    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {        
		logError("error displaying message box");
		return;
    }

    if (buttonid == -1)
	{
        logDebug("no selection");
		CustomEvents::MessageBoxCallback(Code, 0); // 0 - Cancel
    }
    else 
	{        
		CustomEvents::MessageBoxCallback(Code, buttonid + 1); // msgBox::Action Button[n]
    }
};
