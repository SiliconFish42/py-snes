#!/usr/bin/env python3
"""
Test cleanup utility for PySNES

This script cleans up old test outputs and reports based on configuration.
"""

import argparse
import json
import shutil
import sys
from datetime import datetime, timedelta
from pathlib import Path


def load_config() -> dict:
    """Load test configuration"""
    config_file = Path("tests/config/test_config.json")
    if not config_file.exists():
        print("❌ tests/config/test_config.json not found")
        sys.exit(1)
    
    with open(config_file) as f:
        return json.load(f)


def cleanup_old_reports(output_dir: Path, max_age_days: int = 7) -> None:
    """Clean up old test reports"""
    if not output_dir.exists():
        print(f"ℹ️  Output directory {output_dir} does not exist")
        return
    
    cutoff_date = datetime.now() - timedelta(days=max_age_days)
    removed_count = 0
    
    for file_path in output_dir.iterdir():
        if file_path.is_file():
            file_age = datetime.fromtimestamp(file_path.stat().st_mtime)
            if file_age < cutoff_date:
                file_path.unlink()
                removed_count += 1
                print(f"🗑️  Removed old file: {file_path.name}")
    
    if removed_count > 0:
        print(f"✅ Cleaned up {removed_count} old files")
    else:
        print("ℹ️  No old files to clean up")


def cleanup_all_test_outputs() -> None:
    """Clean up all test outputs"""
    output_dir = Path("test_output")
    if output_dir.exists():
        shutil.rmtree(output_dir)
        print("🗑️  Removed all test outputs")
    else:
        print("ℹ️  No test outputs to clean up")


def main():
    parser = argparse.ArgumentParser(description="Clean up PySNES test outputs")
    parser.add_argument("--all", action="store_true", 
                       help="Remove all test outputs")
    parser.add_argument("--old", action="store_true",
                       help="Remove old test reports based on config")
    parser.add_argument("--days", type=int, default=None,
                       help="Override max age in days for old reports")
    
    args = parser.parse_args()
    
    if not args.all and not args.old:
        parser.print_help()
        return
    
    try:
        if args.all:
            cleanup_all_test_outputs()
        elif args.old:
            config = load_config()
            output_dir = Path(config.get("reporting", {}).get("output_directory", "test_output"))
            max_age = args.days or config.get("reporting", {}).get("max_report_age_days", 7)
            cleanup_old_reports(output_dir, max_age)
    
    except Exception as e:
        print(f"❌ Cleanup failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main() 