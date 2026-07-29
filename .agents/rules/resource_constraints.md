# Machine Resource Constraints & Thread Limits

On this specific machine, system resources are severely limited. Running too many threads can cause the system to crash.

## Rules & Guidelines

1. **Thread Limit**: Limit running threads to at most 2, including Antigravity threads.
2. **Single-Threaded Preference**: Prefer single-threaded processes and executions wherever possible.
3. **C++ Builds**: All C++ builds must be single-threaded.
   - Do NOT use parallel compilation flags (e.g., do not use `-j` or `-jN` with `make` or `ninja`, unless explicitly specifying `-j1`).
   - Use `make -j1` or simply `make`.
4. **Java Builds**: All Java builds must be single-threaded.
   - Do NOT run Maven or Gradle builds in parallel mode.
   - Do NOT use `-T` or `--threads` with Maven.
