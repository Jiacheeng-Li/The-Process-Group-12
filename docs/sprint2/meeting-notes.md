# Team Meeting Minutes - Sprint 3 Planning Session

## Meeting Details
- **Meeting Topic**: Sprint 3 Goal Definition & Task Refinement
- **Date**: November 21th, 2025
- **Attendees**: JC, CN, ZY, XT, YX (All Members)
- **Absent**: None

## 1. Sprint 2 Review & Summary

### Achievements:
- Completed unified UI design language and project structure setup
- Implemented enhanced video player and home page video feed
- Built application navigation framework and static social interfaces
- Implemented video shooting flow and preliminary accessibility design (high contrast mode, responsive)
- Synchronously updated process documentation and ethical materials

### Learnings & Reflections:
- Early unified design language significantly improved development consistency
- Early integration of accessibility features noticeably enhanced user experience
- Simulated data strategy effectively supported rapid iteration during prototype phase

## 2. Sprint 3 Overall Goal

**Core Focus**: "Deepen Accessibility Design and User Experience, Perfect Core Functions, and Prepare Final Delivery Materials"

This iteration will build upon Sprint 2 to further implement **Universal Usability**, improve video playback and interaction experience, while completing all documentation, videos, and code integration required for the final presentation.

## 3. Role Assignment & Sprint Backlog Task Breakdown

| Task Title & Description | Owner | Specific Deliverables & Acceptance Criteria |
|-------------------------|-------|---------------------------------------------|
| T1: Implement Enhanced Video Player Features (Subtitles, Full-screen Optimization, Progress Control) | ZY | **Deliverables:**<br>1. Video player supporting external subtitles<br>2. Full-screen playback experience optimization<br>3. Progress bar supporting dragging and preview<br>**Acceptance Criteria:** Users can enable subtitles in full-screen mode and smoothly control playback progress |
| T2: Deepen Accessibility Features (Contrast Adjustment, Font Scaling, Icon Labels) | ZY | **Deliverables:**<br>1. Adjustable contrast settings interface and implementation<br>2. Global font scaling functionality<br>3. Add text labels beside icons to improve recognizability<br>**Acceptance Criteria:** Users can adjust contrast and font size in settings; all icons have corresponding text descriptions |
| T3: Code Integration, UI Unification and Project Restructuring | YX | **Deliverables:**<br>1. Code structure optimization, removing redundancy<br>2. Unified global UI components style<br>3. Ensure all pages follow design specifications<br>**Acceptance Criteria:** High code readability, consistent UI across different pages |
| T4: Write Final Project Report and User Evaluation Materials | CN | **Deliverables:**<br>1. Prepare content for final project report (including iteration process, evaluation results, function descriptions)<br>2. User evaluation questionnaire design and result analysis<br>3. Final demonstration script<br>4. Final version of ethics statement and user consent forms<br>**Acceptance Criteria:** Clear report structure, complete content, meeting course requirements |
| T5: Video Production and Presentation Material Preparation | JC & XT | **Deliverables:**<br>1. Product demonstration video (including feature showcase and accessibility features explanation)<br>2. Kanban maintenance, complete meeting notes, update Gantt chart<br>**Acceptance Criteria:** Video smoothly demonstrates core functions, all materials are properly formatted |

## 4. Technical Feasibility Discussion & Risk Mitigation

### Challenges:
- Subtitle parsing and synchronous rendering may involve complex timing control
- Global font scaling may affect some layout adaptations

### Decisions & Solutions:
1. **Subtitle Implementation Solution**: Use `.srt` format subtitles, implement rendering using `QMediaPlayer` combined with `QGraphicsTextItem`, avoiding complex parsing libraries.
2. **Font Scaling Adaptation**: Use `QFontMetrics` for dynamic layout calculation, ensuring text scaling doesn't affect interface structure.
3. **Testing Strategy**: Conduct cross-testing within the team upon completion of each accessibility feature to ensure functionality works and doesn't affect existing interactions.

## 5. Success Criteria & Evaluation Preparation

To prepare for the final review, we define the following success criteria for Sprint 3:

- **Functional Completeness**: All planned accessibility features (subtitles, contrast, font scaling, icon labels) are usable
- **User Experience Consistency**: Unified style across all pages with smooth interactions
- **Documentation Completeness**: Final report, demonstration video, ethical materials are complete and consistent in content
- **Code Deliverability**: Clear code structure, complete comments, compilable and runnable in new environments

## 6. Next Action Plan

1. **Immediate Action**: ZY completes technical research and implementation plan for subtitle functionality within Sprint 3
2. **Kanban Update**: XT breaks down the above tasks into subtasks and enters them into the **Sprint 3 Backlog**
3. **Development Sync**: YX and ZY proceed with code integration and feature development in parallel, while CN simultaneously writes the report structure
4. **Daily Stand-ups**: Continue daily synchronization in WeChat group, with XT recording progress and blocking issues

**Next Planning Meeting**: Before the end of Sprint 3 (November 28th, 2025), for review and retrospective.

---

*Minutes Recorded By: XT*