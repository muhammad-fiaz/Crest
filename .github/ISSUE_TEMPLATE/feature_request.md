---
name: Feature Request
description: Suggest a new feature or enhancement for Crest
title: "[FEATURE] "
labels: ["enhancement", "feature-request"]
assignees: []
body:
  - type: markdown
    attributes:
      value: |
        ## Feature Request
        
        Thank you for suggesting a new feature! Your ideas help make Crest better for everyone.

  - type: textarea
    id: summary
    attributes:
      label: Feature Summary
      description: Provide a brief summary of the feature you'd like to see.
      placeholder: "A clear and concise description of the proposed feature."
    validations:
      required: true

  - type: textarea
    id: problem
    attributes:
      label: Problem/Use Case
      description: What problem would this feature solve? What's the use case?
      placeholder: "Describe the problem this feature would address or the use case it would enable."
    validations:
      required: true

  - type: textarea
    id: solution
    attributes:
      label: Proposed Solution
      description: Describe your proposed solution or implementation approach.
      placeholder: "How would you like this feature to work? Include any API design ideas."
    validations:
      required: true

  - type: textarea
    id: alternatives
    attributes:
      label: Alternative Solutions
      description: Have you considered any alternative approaches?
      placeholder: "Describe any alternative solutions or workarounds you've considered."

  - type: dropdown
    id: priority
    attributes:
      label: Priority
      description: How important is this feature to you?
      options:
        - Nice to have
        - Would be helpful
        - Important for my use case
        - Critical/blocking my work
    validations:
      required: true

  - type: textarea
    id: additional-context
    attributes:
      label: Additional Context
      description: Add any other context, screenshots, or examples.
      placeholder: "Links to similar features in other projects, code examples, etc."

  - type: checkboxes
    id: checklist
    attributes:
      label: Checklist
      description: Please check the following before submitting
      options:
        - label: I have searched for similar feature requests and couldn't find any
          required: true
        - label: This feature would benefit other users, not just me
          required: true
        - label: I understand this is a suggestion and may not be implemented
          required: true