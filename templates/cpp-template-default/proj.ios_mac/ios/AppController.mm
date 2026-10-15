/****************************************************************************
 Copyright (c) 2010-2013 cocos2d-x.org
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#import "AppController.h"
#import "cocos2d.h"
#import "AppDelegate.h"
#import "RootViewController.h"

@interface AppController ()
- (UIWindow *)createWindowForScene:(UIWindowScene *)scene;
- (void)sceneDidEnterBackground;
- (void)sceneWillEnterForeground;
@end

@interface SceneDelegate : UIResponder <UIWindowSceneDelegate>
@end

@implementation AppController
{
    BOOL _started;
    BOOL _inBackground;
}

#pragma mark -
#pragma mark Application lifecycle

// cocos2d application instance
static AppDelegate s_sharedApplication;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    cocos2d::Application *app = cocos2d::Application::getInstance();
    
    // Initialize the GLView attributes
    app->initGLContextAttrs();
    cocos2d::GLViewImpl::convertAttrs();
    
    //Launching the app with the arguments -NSAllowsDefaultLineBreakStrategy NO to force back to the old behavior.
    if ( [[UIDevice currentDevice].systemVersion floatValue] >= 13.0f)
    {
        [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSAllowsDefaultLineBreakStrategy"];
    }

    return YES;
}

- (UIWindow *)createWindowForScene:(UIWindowScene *)scene {
    UIWindow *window = [[UIWindow alloc] initWithWindowScene:scene];

    // The scene may be discarded in background and connected again later: the game keeps running
    // and only moves into the new window.
    if (!_viewController)
    {
        // Use RootViewController to manage CCEAGLView
        _viewController = [[RootViewController alloc]init];
        _viewController.edgesForExtendedLayout = UIRectEdgeTop | UIRectEdgeBottom;
        _viewController.extendedLayoutIncludesOpaqueBars = YES;
    }

    [window setRootViewController:_viewController];
    [window makeKeyAndVisible];

    if (!_started)
    {
        _started = YES;

        // IMPORTANT: Setting the GLView should be done after creating the RootViewController
        cocos2d::GLView *glview = cocos2d::GLViewImpl::createWithEAGLView((__bridge void *)_viewController.view);
        cocos2d::Director::getInstance()->setOpenGLView(glview);

        //run the cocos2d-x game scene
        cocos2d::Application::getInstance()->run();
    }

    return window;
}

- (void)sceneDidEnterBackground {
    _inBackground = YES;
    cocos2d::Application::getInstance()->applicationDidEnterBackground();
}

- (void)sceneWillEnterForeground {
    // Unlike applicationWillEnterForeground, this also arrives on cold start, when there is nothing to resume.
    if (!_inBackground)
        return;

    _inBackground = NO;
    cocos2d::Application::getInstance()->applicationWillEnterForeground();
}

- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


#if __has_feature(objc_arc)
#else
- (void)dealloc {
    [_viewController release];
    [super dealloc];
}
#endif


@end


@implementation SceneDelegate

@synthesize window;

- (AppController *)appController {
    return (AppController *)[UIApplication sharedApplication].delegate;
}

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    if (![scene isKindOfClass:[UIWindowScene class]])
        return;

    UIWindow *created = [[self appController] createWindowForScene:(UIWindowScene *)scene];
    self.window = created;
#if !__has_feature(objc_arc)
    [created release];
#endif
}

- (void)sceneDidDisconnect:(UIScene *)scene {
    self.window.rootViewController = nil;
    self.window = nil;
}

- (void)sceneDidEnterBackground:(UIScene *)scene {
    [[self appController] sceneDidEnterBackground];
}

- (void)sceneWillEnterForeground:(UIScene *)scene {
    [[self appController] sceneWillEnterForeground];
}

#if __has_feature(objc_arc)
#else
- (void)dealloc {
    [window release];
    [super dealloc];
}
#endif

@end
