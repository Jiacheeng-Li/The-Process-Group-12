# Team Meeting Minutes - Sprint 2 Planning Session

## Meeting Details
- **Meeting Topic**: Sprint 2 Goal Definition & Task Decomposition
- **Date**: November 14th, 2025
- **Attendees**: JC, CN, ZY, XT, YX (All Members)
- **Absent**: None

## 1. Sprint 1 Review & Summary

### Achievements:
- Successfully ran and analyzed the Tomeo starter code, understanding core classes like `the_player`, `the_button`
- Established team collaboration infrastructure: GitHub repository, Kanban board, communication channels
- Fixed stability issues in the base code, ensuring a runnable prototype

### Learnings & Reflections:
- Gained initial practical experience with the Qt multimedia framework and C++ signals and slots mechanism
- Recognized the need to establish unified code style and UI design guidelines earlier to ensure consistency in future iterations

## 2. Sprint 2 Overall Goal

**Core Focus**: "Build an Interactive Prototype Framework with Consistent Visual Language and Core User Experience"

This sprint will evolve the project from an unstable tech demo into a **Minimum Viable Product** with a clear structure, unified design, and core video playback and content creation workflows. We place special emphasis on the early integration of **Universal Usability**.

## 3. Role Assignment & Sprint Backlog Task Breakdown

| Task Title & Description | Owner | Specific Deliverables & Acceptance Criteria |
|-------------------------|-------|---------------------------------------------|
| T1: Establish Global UI/UX Design Language & Project Structure | JC | **Deliverables:**<br>1. UI Style Guide (Color, Typography, Spacing, Iconography)<br>2. High-fidelity wireframes/mockups for all core pages<br>3. Clear, standardized GitHub repository structure<br>**Acceptance Criteria:** All team members can develop independently based on this guide; interface style is consistent |
| T2: Implement Enhanced Video Player & Home Page | CN | **Deliverables:**<br>1. Stable, fully functional full-screen video player (Play/Pause, Seek Bar, Volume)<br>2. Optimized Home page video feed with smooth scrolling<br>**Acceptance Criteria:** Users can seamlessly enter full-screen mode from the Home page and perform complete playback control; no crashes |
| T3: Implement Application Navigation Framework & Social Interfaces | ZY | **Deliverables:**<br>1. Bottom Navigation Bar (Home, Capture, Friends, Profile)<br>2. Static implementation of Profile Page (displaying avatar, nickname, video list)<br>3. Static implementation of Chat Page (simulated conversation list)<br>**Acceptance Criteria:** The app has clear navigation; all pages are routable; UI components adhere to JC's design specifications |
| T4: Implement Video Capture Flow & Accessibility Design | YX | **Deliverables:**<br>1. Video Capture Interface (Camera preview, record button, interaction feedback)<br>2. Post-capture preview screen<br>3. Initial implementation of **High Contrast Mode** and **Scalable Fonts**<br>**Acceptance Criteria:** Users can complete the full front-end flow from opening the front camera to preview; can enable High Contrast Mode in settings |
| T5: Synchronously Update Process Documentation & Ethical Materials | XT | **Deliverables:**<br>1. Updated Use Case Specification<br>2. These meeting minutes and subsequent stand-up notes<br>3. Real-time task tracking on the Kanban board with screenshots<br>4. Draft Ethics Statement and Informed Consent Forms<br>**Acceptance Criteria:** Documentation is synchronized with development progress, providing a complete, timely evidence trail for the final report |

## 4. Technical Feasibility Discussion & Risk Mitigation

### Challenge:
Real backend social features, video cloud upload, and processing are not feasible for the prototype stage.

### Decisions & Solutions:
1. **Data Simulation Strategy**: All user data, video content, and chat messages will use local static or simulated data. Actions like liking and following will only take effect in the local UI.
2. **Feature Simulation Strategy**: The "Share" function will trigger a simulated system share menu; "Publishing" a video will simulate an upload process, eventually adding the video to the local list.
3. **Focus on Front-end Value**: This cycle focuses on creating an **excellent front-end interaction experience**, demonstrating the product's core value through refined animations, clear processes, and reliable accessibility support, which fully meets the requirements for a "proof-of-concept prototype."

## 5. Success Criteria & Evaluation Preparation

To prepare for the "Experiments/Iterations" scoring, we defined measurable success criteria for the Sprint 2 outcomes:

- **Functional Stability**: All core navigation paths (Home → Playback → Profile → Back) are crash-free
- **Task Completion**: 90% of test users can complete the core tasks of "finding a video and playing it in full screen" and "initiating the video recording process" without guidance
- **Design Consistency**: The visual style and UI components are consistent across all pages
- **Evidence Collection**: XT is prepared to collect Kanban screenshots, meeting notes, and design documents as evidence of the iterative process

## 6. Next Action Plan

1. **Immediate Action**: JC provides the basic UI Style Guide and visual mockups for the Home and Player pages within 24 hours
2. **Kanban Update**: XT refines the above tasks, enters them into the **Sprint 2 Backlog**, and creates subtask checklists
3. **Development Sync**: CN, YX and ZY work in parallel; CN develops the player based on the mockups, while ZY builds the navigation framework. YX builds up the Video shooting interface
4. **Daily Stand-ups**: Hold syncs at Wechat group daily, with XT recording any blockers

**Next Planning Meeting**: Before the end of Sprint 2 (November 21th, 2025), for review and retrospective.

---

*Minutes Recorded By: XT*
