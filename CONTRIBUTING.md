# Contributing to Crest

Thank you for your interest in contributing to Crest! We welcome contributions from the community.

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with:
- A clear title and description
- Steps to reproduce
- Expected vs actual behavior
- Your environment (OS, compiler version, etc.)

### Suggesting Features

We love new ideas! Please create an issue with:
- Clear description of the feature
- Use cases and benefits
- Possible implementation approach

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/YourFeature`)
3. Make your changes
4. Add tests if applicable
5. Ensure all tests pass
6. Commit with clear messages
7. Push to your fork
8. Open a Pull Request

### Code Style

- Use consistent indentation (4 spaces)
- Follow existing code style
- Add comments for complex logic
- Document all public APIs
- Keep functions focused and small

### Testing

- Add tests for new features
- Ensure existing tests pass
- Test on multiple platforms when possible

## Development Setup

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
mkdir build && cd build
cmake .. -DCREST_BUILD_TESTS=ON
cmake --build .
ctest
```

## Code of Conduct

Be respectful, inclusive, and collaborative. We're all here to build something great together!

## Questions?

Feel free to open an issue or reach out to the maintainers.

Thank you for contributing! 🎉
