CREATE TABLE IF NOT EXISTS sessions (
    session_id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_name TEXT NOT NULL,
    start_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    end_time DATETIME,
    working_directory TEXT DEFAULT '/home',
    session_status TEXT DEFAULT 'ACTIVE' CHECK(session_status IN ('ACTIVE', 'CLOSED')),
    total_commands INTEGER DEFAULT 0,
    notes TEXT
);

CREATE TABLE IF NOT EXISTS command_history (
    command_id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id INTEGER NOT NULL,
    user_input TEXT NOT NULL,
    generated_command TEXT NOT NULL,
    execution_status TEXT DEFAULT 'EXECUTED' CHECK(execution_status IN ('EXECUTED', 'FAILED', 'BLOCKED')),
    exit_code INTEGER,
    command_output TEXT,
    error_output TEXT,
    execution_time_ms REAL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    is_safe BOOLEAN DEFAULT 1,
    safety_reason TEXT,
    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS command_cache (
    cache_id INTEGER PRIMARY KEY AUTOINCREMENT,
    command_hash TEXT UNIQUE NOT NULL,
    original_command TEXT NOT NULL,
    llm_generated_command TEXT NOT NULL,
    generated_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    usage_count INTEGER DEFAULT 1,
    last_used DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS system_logs (
    log_id INTEGER PRIMARY KEY AUTOINCREMENT,
    log_level TEXT CHECK(log_level IN ('INFO', 'WARNING', 'ERROR', 'DEBUG')),
    log_message TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    component TEXT,
    session_id INTEGER,
    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS performance_metrics (
    metric_id INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id INTEGER NOT NULL,
    command_id INTEGER,
    llm_response_time_ms REAL,
    safety_check_time_ms REAL,
    command_execution_time_ms REAL,
    total_time_ms REAL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,
    FOREIGN KEY (command_id) REFERENCES command_history(command_id) ON DELETE CASCADE
);

CREATE INDEX idx_sessions_start_time ON sessions(start_time);
CREATE INDEX idx_sessions_status ON sessions(session_status);
CREATE INDEX idx_command_history_session ON command_history(session_id);
CREATE INDEX idx_command_history_timestamp ON command_history(timestamp);
CREATE INDEX idx_command_history_status ON command_history(execution_status);
CREATE INDEX idx_command_cache_hash ON command_cache(command_hash);
CREATE INDEX idx_system_logs_timestamp ON system_logs(timestamp);
CREATE INDEX idx_performance_metrics_session ON performance_metrics(session_id);
