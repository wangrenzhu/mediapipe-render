//
//  ViewController.m
//  OpipeBeautyModuleExample
//
//  Created by 王韧竹 on 2022/9/8.
//

#import "ViewController.h"
#import "GLRenderViewController.h"
#import <OlaFaceUnityFramework/OlaFaceUnity.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (IBAction)beaudyTouchUp:(UIButton *)sender {
    if (sender.tag == 1) {
        [OlaFaceUnity sharedInstance].useNewBeauty = YES;
    } else {
        [OlaFaceUnity sharedInstance].useNewBeauty = NO;
    }
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
