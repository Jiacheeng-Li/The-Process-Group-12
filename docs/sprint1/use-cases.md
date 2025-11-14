# Antipode: DayDreamNight — Use Case Specification

This document describes the core use cases for the **Antipode: DayDreamNight** video social prototype.  

---

# UC-01: App Launch & Antipode Initialisation

## **Goal**
Obtain user location, compute antipode coordinates, and load recommended videos.

## **Primary Actor**
User

## **Preconditions**
- App installed
- User has not disabled location permissions

## **Main Flow**
1. User opens the app.
2. System requests permission to access location.
3. User grants permission.
4. A rotating Earth animation appears.
5. System calculates the antipode:  
   **lat → -lat**, **lon → lon ± 180°**.
6. System pins the user location and antipodal point on the Earth model.
7. Home feed loads videos recorded in the antipodal region.

## **Alternative / Exception Flows**
- **A1: User denies location permission** → Load global trending videos instead.
- **A2: No internet** → Load cached videos and display an offline banner.
- **A3: Location error** → Prompt to retry or manually input region.

---

# UC-02: Browse & Interact with Recommended Videos

## **Goal**
Allow users to browse, view, and interact with videos from their antipode.

## **Primary Actor**
User

## **Main Flow**
1. Home page displays a full-screen video card.
2. User swipes **up/down** to move between videos.
3. Floating action buttons appear: ❤️ Like, 💬 Comment, ↗ Share.
4. User taps ❤️ → Like count increases with animation.
5. User taps 💬 → Comment sheet slides up; user can add or view comments.
6. User taps ↗ → System opens share menu.
7. User taps username or swipes left → Navigate to Profile page.

## **Alternative Flows**
- **A1: Video load failed** → Show retry button.
- **A2: No comments available** → Display “Be the first to comment”.

---

# UC-03: View Another User’s Profile

## **Goal**
Display a user's public information and enable social actions.

## **Primary Actor**
User

## **Main Flow**
1. User enters profile via swipe-left or tapping username.
2. System shows profile header (avatar, nickname, bio).
3. System displays Pinned videos at the top.
4. Below shows a list of recent uploads.
5. User taps **Follow** to follow the creator.
6. User taps **Message** to open Chat window.

## **Alternative Flows**
- **A1: Private profile** → Show limited preview and request approval to follow.
- **A2: Not logged in** → System prompts login before messaging.

---

# UC-04: Chat & Social Messaging

## **Goal**
Allow users to communicate with friends in real time.

## **Primary Actor**
User

## **Main Flow**
1. User opens Chat page.
2. A session list shows all active conversations.
3. User selects a friend → Enters chat view.
4. User sends a message (text or emoji).
5. Message appears instantly in chat.
6. System marks message as delivered/seen (simulated in prototype).

## **Alternative Flows**
- **A1: Network loss** → System buffers outgoing messages.
- **A2: Blocked user** → Messaging option disabled.

---

# UC-05: Record & Upload Video

## **Goal**
Allow users to create and upload one short video per day.

## **Primary Actor**
User

## **Preconditions**
- Camera permission granted
- Daily upload limit not exceeded

## **Main Flow**
1. User taps the center **Record** button.
2. Record page opens with front/back camera toggle.
3. User taps **Record** to start capturing (default 10 seconds).
4. User taps **Stop** → Preview screen appears.
5. User adds tags, description, and location info.
6. User taps **Upload** → Upload progress bar appears.
7. Upload successful → Video appears on Home and Profile.

## **Alternative Flows**
- **A1: Recording fails** → Show "retry recording" prompt.
- **A2: Upload fails** → Save as draft for later retry.

---

# UC-06: Manage Personal Profile & Pin Videos

## **Goal**
Enable users to customise their profile and curate highlighted content.

## **Primary Actor**
User

## **Main Flow**
1. User opens Profile page.
2. User taps **Edit profile** to change avatar, nickname, or bio.
3. A list of all uploaded videos is shown.
4. User taps **Pin** to place a video at the top of the profile.
5. System updates the pinned video order instantly.
6. “Today’s Video” section displays the current day’s upload.

## **Alternative Flows**
- **A1: No pinned videos** → System displays default placeholder.
- **A2: Multiple pinned items** → Allow reordering by long-press and drag.

---

# UC-07: Earth Model Interaction (Optional for prototype)

## **Goal**
Provide a visual and interactive representation of global locations.

## **Primary Actor**
User

## **Main Flow**
1. A 3D/globe widget appears at the bottom of Home.
2. Earth rotates to align with the user's location.
3. Antipode location is marked with a pin.
4. User clicks/hover pin → Shows region info (city, distance, timezone).
5. User taps → Opens video feed for that region.

## **Alternative Flows**
- **A1: 3D model disabled (performance)** → Use static 2D map.

---

# Summary Table

| Use Case | Description |
|---------|-------------|
| UC-01 | App launch & location/antipode initialisation |
| UC-02 | Browse and interact with videos |
| UC-03 | View another user’s profile |
| UC-04 | Messaging & chat |
| UC-05 | Record and upload video |
| UC-06 | Manage profile & pin videos |
| UC-07 | Earth model visual interaction |

---

# Document Status
**Version:** Sprint 1  
**Author:** Group 12 — Jiacheng Li 
**Status:** Approved for Sprint 2 development
