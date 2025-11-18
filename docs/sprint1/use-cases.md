# Antipode: DayDreamNight — Use Case Specification

This document describes the core use cases for the **Antipode: DayDreamNight** video social prototype.  

---

# UC-01: App Launch & Antipode Initialisation

**Goal:** Successfully obtain user location, intuitively demonstrate the antipode concept, and load relevant video content, creating a "wow" first impression for users.

**Primary Actor:** User

**Priority:** High

**Design Rationale:** Nielsen's Heuristics - Visibility of System Status (using animations and map markers to inform users about system processes and geographical relationships).

**Success Criteria:** 
- users understand the antipode concept on first launch
- Location initialization process completes in <5 seconds on average

**Preconditions:**
- App is installed
- User has not permanently denied location permissions

**Main Flow:**
1. User opens the app
2. System displays an elegant splash screen with App Logo
3. System requests location permission via a dialog explaining why it's needed
4. User grants permission
5. System displays a rotating Earth animation, gradually zooming to the user's approximate region
6. System calculates antipode: `latitude → -latitude`, `longitude → longitude ± 180°`
7. System clearly marks user location and antipode on the Earth model using two distinct pins (e.g., blue and red), displaying a dotted line connecting them
8. Home feed begins loading video previews recorded in the antipodal region

**A1: User denies location permission**
1. System displays message: "No location available. Showing trending global videos instead."
2. Home feed loads globally popular videos

**A2: No internet connection**
1. System displays offline banner
2. Attempts to load any cached video content

**A3: Location acquisition times out or fails**
1. System prompts: "Location failed. Check network/GPS settings or select your region manually."
2. Provides a continent/country list for manual selection

**Evaluation Methods:** Cognitive Walkthrough, First-Time User Experience Questionnaire

# UC-02: Browse & Interact with Recommended Videos

**Goal:** Provide a fluid, immersive, and intuitive video browsing and interaction experience to maximize user engagement.

**Primary Actor:** User

**Priority:** High

**Design Rationale:**
- **Fitts' Law:** Places core interaction buttons (Like, Comment, Share) in the easily reachable bottom-right screen area
- **Jakob's Law:** Adopts the familiar vertical swipe interaction pattern (similar to TikTok/Instagram) to reduce learning curve
- **Accessibility:** Supports keyboard navigation with arrow keys

**Main Flow:**
1. Home page auto-plays a full-screen video (muted by default)
2. User vertically swipes to navigate to previous/next video
3. Semi-transparent action buttons overlay video on right: ❤️ (Like), 💬 (Comment), ↗ (Share), user avatar
4. User taps ❤️ → Button fills red, like count increments +1, with subtle "heart" pop animation
5. User taps 💬 → A modal comments sheet slides up from bottom, showing existing comments and input field
6. User taps ↗ → Triggers native system share menu with options to share to other apps
7. User taps user avatar → Navigates to that user's Profile page (UC-03)

**Alternative Flows:**

**A1: Video fails to load**
- Displays a placeholder graphic and prominent "Retry" button

**A2: No comments available**
- Comments area shows encouraging text: "No comments yet. Be the first to comment!"

**Success Criteria:**
- Users can successfully swipe to browse without guidance
- 95% of interaction attempts (like, comment) succeed on first try

**Evaluation Methods:** Heuristic Evaluation, Usability Testing (observing natural user behavior)

---

# UC-03: View Another User's Profile 

## **Goal**
Enable users to explore creator profiles, understand their content style, and initiate social connections in an intuitive and engaging manner.

## **Primary Actor**
User

## **Priority**
Medium

## **Design Rationale**
- **Information Hierarchy** (Visual weight to prioritize pinned content)
- **Progressive Disclosure** (Show basic info first, detailed stats on demand)
- **Social Proof** (Display follower counts and engagement metrics)
- **Accessibility** (Ensure profile is navigable via keyboard and screen readers)

## **Success Criteria**
- 90% of users can successfully follow/unfollow within 3 seconds
- Profile pages load completely within 2 seconds
- Users can understand creator's content focus within 15 seconds of viewing profile

## **Preconditions**
- User is authenticated
- Target profile exists and is accessible
- Network connection available

## **Main Flow**
1. User taps on a creator's username or avatar from any context (video feed, comments, search)
2. System displays profile loading skeleton screen
3. System loads and displays profile header containing:
   - Large profile avatar
   - Display name and username
   - Bio/description text
   - Follower/Following counts
   - "Follow" button (state depends on current relationship)
4. Below header, system displays:
   - **Pinned Videos Section** (top, with special visual treatment)
   - **Video Grid** of recent uploads (chronological order)
   - **Achievement Badges** (if applicable)
5. User scrolls to explore content
6. User taps **Follow** → Button changes to "Following" with confirmation animation, follower count increments
7. User taps **Message** → System opens chat interface with that user pre-selected (UC-04)

## **Alternative Flows**
- **A1: Private Profile**
  1. System displays limited preview (avatar, username, private account badge)
  2. Follow button shows "Request"
  3. User taps "Request" → System sends follow request, button changes to "Requested"
  4. Creator receives notification to approve/deny request

- **A2: Own Profile**
  1. System displays "Edit Profile" button instead of "Follow"
  2. Additional options: "Settings", "Analytics", "Drafts"
  3. Pinned videos can be reordered via drag-and-drop

- **A3: Network Error**
  1. System displays error state with retry button
  2. Cached profile data displayed if available
  3. Offline indicator shown

## **Evaluation Methods**
- Usability testing for profile navigation efficiency
- A/B testing different profile layouts
- Analytics on follow conversion rates

---

# UC-04: Chat & Social Messaging

## **Goal**
Facilitate authentic conversations between users while maintaining a clean, responsive, and reliable messaging experience.

## **Primary Actor**
User

## **Priority**
Low (for prototype - can be simulated)

## **Design Rationale**
- **Reciprocity Principle** (Read receipts encourage response)
- **Cognitive Load Reduction** (Clear message status, typing indicators)
- **Progressive Enhancement** (Basic functionality works offline)
- **Accessibility** (Full keyboard navigation, screen reader support)

## **Success Criteria**
- Message delivery feels instantaneous (<100ms visual feedback)
- 95% of messages successfully sent on first attempt
- Users can navigate entire chat interface using keyboard only

## **Preconditions**
- User is authenticated
- Both users have established connection (mutual follow)
- Chat feature is enabled for both accounts

## **Main Flow**
1. User navigates to Messages tab from bottom navigation
2. System displays conversation list, sorted by most recent activity
3. Each conversation preview shows:
   - Friend's avatar and name
   - Last message preview and timestamp
   - Unread message indicator (if applicable)
4. User taps a conversation → System opens chat view with:
   - Message history (scrollable)
   - Message input field at bottom
   - Online status indicator for friend
5. User types message and presses Send/Secure Send:
   - Message appears immediately in chat bubble
   - System shows "Sending..." status
   - On successful send: status changes to "Delivered" ✓
   - When friend views: status changes to "Seen" 👁️
6. Friend's responses appear in real-time with notification

## **Alternative Flows**
- **A1: Network Connectivity Issues**
  1. System detects poor/no connectivity
  2. Outgoing messages queue with "Waiting for network" indicator
  3. Automatic retry when connection restored
  4. User can manually retry failed sends

- **A2: Blocked User**
  1. System prevents message sending to blocked users
  2. Clear messaging: "You cannot message this user"
  3. Option to unblock from profile page

- **A3: Long Message Composition**
  1. System supports multi-line text input with auto-expand
  2. Character count indicator for long messages
  3. Draft auto-save every 30 seconds

- **A4: Media Sharing (Extended)**
  1. User taps attachment button → options: Photo, Video, Location
  2. System handles file upload with progress indicator
  3. Recipient sees media preview in chat

## **Evaluation Methods**
- Message success rate monitoring
- User satisfaction surveys on chat experience
- Performance testing under poor network conditions

---

# UC-05: Record & Upload Video

**Goal:** Enable users to quickly and easily complete their daily short video creation and sharing, encouraging content production.

**Primary Actor:** User

**Priority:** Medium

**Design Rationale:**
- **Constrained Design:** "Once per day" upload limit reduces decision fatigue and mimics BeReal's core mechanic, encouraging authenticity
- **Error Prevention:** Providing preview and editing steps before upload prevents users from publishing unsatisfactory content

**Preconditions:**
- Camera and microphone permissions granted
- User has not uploaded a video today

**Main Flow:**
1. User taps prominent + (Record) button in center of bottom navigation bar
2. Recording interface opens full-screen, defaulting to rear camera, with clean UI (camera flip button top-right)
3. User long-presses central red circle record button to start capture (UI provides 3-10 second countdown visual feedback)
4. User releases button or maximum duration reached → Recording stops, immediately transitions to preview screen
5. On preview screen, user can:
   - a. Re-record
   - b. Add description text
   - c. Add location tag (auto-populated with antipode location, editable)
6. User taps Publish → Interface shows upload progress indicator
7. Upon success, system shows confirmation, automatically returns to Home, user's new video appears at top of feed

**Alternative Flows:**

**A1: Recording fails (e.g., permissions revoked)**
- Shows prompt: "Cannot access camera. Please grant permission in System Settings"

**A2: Upload fails**
- Shows prompt: "Upload failed. Network connection issue"
- Provides "Retry" button and "Save as Draft" option

**Success Criteria:**
- Average task completion time (start recording to successful publish) <60 seconds
- Abandonment rate in publish flow <10%

**Evaluation Methods:** Usability Testing, Task Success Rate Measurement

---
# UC-06: Manage Personal Profile & Pin Videos

## **Goal**
Empower users to craft their digital identity and showcase their best content through intuitive profile customization tools.

## **Primary Actor**
Authenticated User

## **Priority**
Medium

## **Design Rationale**
- **Self-Expression Theory** (Allow personalization for identity formation)
- **Peak-End Rule** (Highlight best content via pinning for positive impression)
- **Consistency Standards** (Maintain visual coherence across platform)
- **Error Prevention** (Confirm destructive actions like bio changes)

## **Success Criteria**
- users feel profile accurately represents their identity

## **Preconditions**
- User is authenticated and viewing own profile
- User has at least one uploaded video (for pinning)

## **Main Flow**
1. User navigates to their profile via bottom navigation
2. System displays profile in "view mode" with Edit button prominently placed
3. User taps **Edit Profile** → System transitions to edit mode with:
   - Avatar change option (camera icon overlay)
   - Editable display name, username, bio fields
   - Link/website field
   - Privacy settings toggle
   - Save/Cancel buttons
4. User makes changes and taps **Save** → System validates and updates profile
5. In profile view, user scrolls to video grid
6. User taps **Pin** icon on any video thumbnail → System:
   - Moves video to dedicated "Pinned" section at top
   - Shows pin animation and confirmation
   - Updates pin counter
7. User long-presses pinned video → Enters reorder mode with drag handles
8. User drags to desired position and releases → System updates order instantly

## **Alternative Flows**
- **A1: No Pinned Videos**
  1. System displays encouraging placeholder: "Pin your best work to feature it here!"
  2. Call-to-action button: "Browse Your Videos"

- **A2: Maximum Pins Reached**
  1. System displays limit message: "Maximum 3 pinned videos"
  2. Offers to replace existing pin or cancel
  3. Clear visual feedback on limit state

- **A3: Edit Conflict**
  1. System detects simultaneous edit from another device
  2. Shows conflict resolution dialog: "Which version would you like to keep?"
  3. Option to compare changes side-by-side

- **A4: Invalid Profile Data**
  1. System validates in real-time as user types
  2. Clear error messages: "Username must be 3-20 characters"
  3. Save button disabled until all validation passes

## **Evaluation Methods**
- Profile completion rate analytics
- Usability testing for pinning workflow
- A/B testing different profile layout options

---

# UC-07: Earth Model Interaction (Optional for prototype)

## **Goal**
Create an engaging, educational spatial visualization that reinforces the core antipode concept and encourages geographical exploration.

## **Primary Actor**
User

## **Priority**
Low (Stretch goal - can be simplified for prototype)

## **Design Rationale**
- **Spatial Learning** (3D visualization enhances geographical understanding)
- **Gamification** (Interactive elements increase engagement)
- **Performance Awareness** (Graceful degradation for lower-end devices)
- **Educational Value** (Teach antipode concept through interaction)

## **Success Criteria**
- Earth model loads and becomes interactive within 3 seconds
- 80% of users correctly identify their antipode after interaction
- Model maintains smooth rotation (≥30fps) on target devices

## **Preconditions**
- Location services enabled and successful
- Sufficient device performance for 3D rendering
- Adequate screen size for meaningful interaction

## **Main Flow**
1. On Home screen, user sees interactive Earth widget (bottom section or corner)
2. Earth automatically rotates to center on user's location
3. Two prominent markers appear:
   - **Blue marker** at user's location (pulsing animation)
   - **Red marker** at calculated antipode (connected by dotted arc)
4. User interacts with model:
   - **Drag** to rotate Earth freely
   - **Pinch** to zoom in/out
   - **Tap marker** → displays tooltip with:
     - City/region name
     - Local time calculation
     - Distance from current location
     - "Explore Videos" button
5. User taps "Explore Videos" → System loads video feed from that region
6. User double-taps empty space → Earth resets to original orientation

## **Alternative Flows**
- **A1: Performance Constraints**
  1. System detects low-end device or performance issues
  2. Automatically switches to **2D Map Mode** with same markers and interactions
  3. Option in settings to manually choose 2D/3D mode

- **A2: Location Unavailable**
  1. System displays Earth in default view (Americas-centered)
  2. Prompts: "Enable location for personalized view"
  3. Major cities pre-marked for exploration

- **A3: Educational Mode**
  1. User long-presses on any location → "Set as reference point"
  2. System calculates and shows that point's antipode
  3. Comparative information displayed (climate, culture facts)

- **A4: Fullscreen Exploration**
  1. User taps expand icon → Earth takes full screen
  2. Enhanced controls: day/night mode, political/physical maps
  3. Search functionality to find specific locations

## **Evaluation Methods**
- User engagement metrics with Earth model
- Geographical knowledge assessment (pre/post interaction)
- Performance benchmarking across different devices

---

## UC-08: Accessibility - Navigate via Keyboard (New)

**Goal:** Ensure users with motor impairments or unable to use touchscreen can fully operate core app features via keyboard.

**Primary Actor:** Keyboard User

**Priority:** Medium

**Design Rationale:** WCAG 2.1 Guidelines, Universal Usability Design

**Preconditions:**
- User uses external keyboard or has assistive keyboard enabled

**Main Flow:**
1. After app launch, keyboard focus automatically lands on first interactive element (e.g., first video card)
2. User cycles through interactive elements (video cards, buttons) using Tab key
3. User navigates video feed using Up/Down arrow keys
4. When focus is on a video, user presses Spacebar to play/pause
5. User presses L key to trigger "Like" function
6. User presses C key to open comments panel
7. User presses Enter key to activate currently focused button or link

**Success Criteria:** All core features (browse, play, like, comment) can be completed independently using keyboard

**Evaluation Methods:** Accessibility Audit, Keyboard Navigation Testing

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
| UC-08 | Accessibility - Navigate via Keyboard|

---

# Document Status
**Version:** Sprint 2
**Status:** Approved for Sprint 2 development
