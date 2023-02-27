#include "nbgl_layout.h"

#define TAP_TOKEN2 0x00
#define BACK_TOKEN 0x01

void nbgl_test_info(uint8_t step_internal);

uint8_t step;
bool drawRect;

static void layoutTouchCallback(int token, uint8_t index) {
  if (token == TAP_TOKEN2) {
    if ((step == 3)||(step ==6)) {
        drawRect = true;
    }
    else {
        drawRect = false;
    }
  }
  else if (token == BACK_TOKEN) {
    if (!drawRect) {
        step -= 2;
    }
  }
  nbgl_test_info(step);
}
void nbgl_test_info(uint8_t step_internal) {
  step = step_internal;
  int layout = 0;
  if (step == 0) {
    // for first step, display first text
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &layoutTouchCallback,
      .tapActionText = "Tap to continue",
      .tapActionToken = TAP_TOKEN2,
      .tapTuneId = NBGL_NO_TUNE,
      .ticker.tickerCallback = NULL,
      .withLeftBorder = true
    };
    if (layout)
      nbgl_layoutRelease(layout);

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddLargeCaseText(layout, "Take Ledger Stax in\nyour hand and hold it\nin a natural position for\nyou.");

    nbgl_layoutDraw(layout);
    step = 1;
  }
  else if (step == 1) {
    // for first step, display first text
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &layoutTouchCallback,
      .tapActionText = "Tap to continue",
      .tapActionToken = TAP_TOKEN2,
      .tapTuneId = NBGL_NO_TUNE,
      .ticker.tickerCallback = NULL,
      .withLeftBorder = true
    };
    if (layout)
      nbgl_layoutRelease(layout);

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 1, true, BACK_TOKEN, NBGL_NO_TUNE);
    nbgl_layoutAddLargeCaseText(layout, "Black squares will\nappear on the screen,\nyou'll have to tap 30 of\nthem.");
    nbgl_layoutDraw(layout);
    step = 2;
  }
  else if (step == 2) {
    // for first step, display first text
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &layoutTouchCallback,
      .tapActionText = "Tap to start",
      .tapActionToken = TAP_TOKEN2,
      .tapTuneId = NBGL_NO_TUNE,
      .ticker.tickerCallback = NULL,
      .withLeftBorder = true
    };
    if (layout)
      nbgl_layoutRelease(layout);

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 1, true, BACK_TOKEN, NBGL_NO_TUNE);
    nbgl_layoutAddLargeCaseText(layout, "This is not a speed\ntest. Don't go too fast\nand try to target the\ncenter of each square.");

    nbgl_layoutDraw(layout);
    step = 3;
  }
  else if (step == 4) {
    // for first step, display first text
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &layoutTouchCallback,
      .tapActionText = "Tap to continue",
      .tapActionToken = TAP_TOKEN2,
      .tapTuneId = NBGL_NO_TUNE,
      .ticker.tickerCallback = NULL,
      .withLeftBorder = true
    };
    if (layout)
      nbgl_layoutRelease(layout);

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddLargeCaseText(layout, "Now change position\nand put down Ledger\nStax on the table.");

    nbgl_layoutDraw(layout);
    step = 5;
  }
  else if (step == 5) {
    // for first step, display first text
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &layoutTouchCallback,
      .tapActionText = "Tap to start",
      .tapActionToken = TAP_TOKEN2,
      .tapTuneId = NBGL_NO_TUNE,
      .ticker.tickerCallback = NULL,
      .withLeftBorder = true
    };
    if (layout)
      nbgl_layoutRelease(layout);

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 1, true, BACK_TOKEN, NBGL_NO_TUNE);
    nbgl_layoutAddLargeCaseText(layout, "Tap 30 more squares\nTo finish the test.");

    nbgl_layoutDraw(layout);
    step = 6;
  }
}